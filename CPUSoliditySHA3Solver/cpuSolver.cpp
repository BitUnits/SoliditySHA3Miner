#include "cpuSolver.h"
#include "sha3.h"
#pragma unmanaged

// --------------------------------------------------------------------
// Static
// --------------------------------------------------------------------

bool cpuSolver::m_pause{ false };

uint32_t cpuSolver::getLogicalProcessorsCount()
{
	return std::thread::hardware_concurrency();
}

std::string cpuSolver::getNewSolutionTemplate(std::string kingAddress)
{
	byte32_t b_solutionTemp;
	std::random_device rand;
	std::mt19937_64 rGen{ rand() };
	std::uniform_int_distribution<uint64_t> uInt_d{ 0, UINT64_MAX };

	for (uint32_t i{ 0u }; i < UINT256_LENGTH; i += UINT64_LENGTH)
		reinterpret_cast<uint64_t&>(b_solutionTemp[i]) = uInt_d(rGen);

	if (kingAddress.empty())
		std::memset(&b_solutionTemp[12], 0, UINT64_LENGTH); // keep first and last 12 bytes, leave middle 8 bytes for mid state 
	else
	{
		address_t king;
		hexStringToBytes(kingAddress, king);

		std::memcpy(&b_solutionTemp[0], &king, ADDRESS_LENGTH); // "King" address
		std::memset(&b_solutionTemp[ADDRESS_LENGTH], 0, UINT64_LENGTH); // mining hash
	}

	return "0x" + bytesToHexString(b_solutionTemp);
}

// --------------------------------------------------------------------
// Public
// --------------------------------------------------------------------

cpuSolver::cpuSolver(std::string const maxDifficulty, std::string const threads) noexcept :
	m_miningThreadCount{ 0u },
	m_hashStartTime{ std::chrono::steady_clock::now() },
	s_address{ "" },
	s_challenge{ "" },
	s_target{ "" },
	s_difficulty{ "" },
	m_address{ 0 },
	m_prefix{ 0 },
	b_target{ 0 },
	m_target{ 0 },
	m_difficulty{ 0 },
	m_maxDifficulty{ maxDifficulty },
	m_solutionHashStartTime{ std::chrono::steady_clock::now() }
{
	const char *delim = (const char *)",";
	char *s_threads = (char *)malloc(threads.size());
	strcpy_s(s_threads, threads.size() + 1, threads.c_str());
	char *nextToken;
	char *token = strtok_s(s_threads, delim, &nextToken);
	while (token != NULL)
	{
		m_miningThreadCount++;
		token = strtok_s(NULL, delim, &nextToken);
	}
	m_miningThreadAffinities = new uint32_t[m_miningThreadCount];
	m_threadHashes = new uint64_t[m_miningThreadCount];
	m_isThreadMining = new bool[m_miningThreadCount];
	memset(m_threadHashes, 0, UINT64_LENGTH * m_miningThreadCount);
	memset(m_isThreadMining, 0, sizeof(bool) * m_miningThreadCount);

	uint32_t threadElement{ 0 };
	strcpy_s(s_threads, threads.size() + 1, threads.c_str());
	nextToken = nullptr;
	token = strtok_s(s_threads, delim, &nextToken);
	while (token != NULL)
	{
		uint32_t threadID;
		byte32_t b_threadID;

		hexStringToBytes(std::string{ token }, b_threadID);
		byte32_t tempThreadID;
		for (uint32_t i{ 0 }; i < UINT256_LENGTH; ++i)
			tempThreadID[i] = b_threadID[UINT256_LENGTH - 1 - i];

		std::memcpy(&threadID, &tempThreadID, UINT32_LENGTH);

		m_miningThreadAffinities[threadElement] = threadID;
		token = strtok_s(NULL, delim, &nextToken);
		threadElement++;
	}
}

cpuSolver::~cpuSolver() noexcept
{
	stopFinding();

	free(m_miningThreadAffinities);
	free(m_isThreadMining);
}

void cpuSolver::setGetKingAddressCallback(GetKingAddressCallback kingAddressCallback)
{
	m_getKingAddressCallback = kingAddressCallback;
}

void cpuSolver::setGetWorkPositionCallback(GetWorkPositionCallback workPositionCallback)
{
	m_getWorkPositionCallback = workPositionCallback;
}

void cpuSolver::setResetWorkPositionCallback(ResetWorkPositionCallback resetWorkPositionCallback)
{
	m_resetWorkPositionCallback = resetWorkPositionCallback;
}

void cpuSolver::setIncrementWorkPositionCallback(IncrementWorkPositionCallback incrementWorkPositionCallback)
{
	m_incrementWorkPositionCallback = incrementWorkPositionCallback;
}

void cpuSolver::setGetSolutionTemplateCallback(GetSolutionTemplateCallback solutionTemplateCallback)
{
	m_getSolutionTemplateCallback = solutionTemplateCallback;
}

void cpuSolver::setMessageCallback(MessageCallback messageCallback)
{
	m_messageCallback = messageCallback;
}

void cpuSolver::setSolutionCallback(SolutionCallback solutionCallback)
{
	m_solutionCallback = solutionCallback;
}

bool cpuSolver::isMining()
{
	for (uint32_t i{ 0 }; i < m_miningThreadCount; ++i)
		if (m_isThreadMining[i])
			return true;

	return false;
}

bool cpuSolver::isPaused()
{
	return cpuSolver::m_pause;
}

void cpuSolver::updatePrefix(std::string const prefix)
{
	assert(prefix.length() == (PREFIX_LENGTH * 2 + 2));

	prefix_t tempPrefix;
	hexStringToBytes(prefix, tempPrefix);

	if (tempPrefix == m_prefix) return;

	s_challenge = prefix.substr(0, 2 + UINT256_LENGTH * 2);
	s_address = "0x" + prefix.substr(2 + UINT256_LENGTH * 2, ADDRESS_LENGTH * 2);

	byte32_t oldChallenge;
	std::memcpy(&oldChallenge, &m_prefix, UINT256_LENGTH);

	std::memcpy(&m_prefix, &tempPrefix, PREFIX_LENGTH);

	onMessage(-1, "Info", "New challenge detected " + s_challenge.substr(0, 18) + "...");
}

void cpuSolver::updateTarget(std::string const target)
{
	if (m_customDifficulty > 0u && !(target == (m_maxDifficulty / m_customDifficulty).GetHex())) return;

	arith_uint256 tempTarget = arith_uint256(target);
	if (tempTarget == m_target) return;

	m_target = tempTarget;
	s_target = (target.substr(0, 2) == "0x") ? target : "0x" + target;

	hexStringToBytes(s_target, b_target);

	onMessage(-1, "Info", "New target detected " + s_target.substr(0, 18) + "...");
}

void cpuSolver::updateDifficulty(std::string const difficulty)
{
	if (m_customDifficulty > 0u) return;

	arith_uint256 oldDifficulity{ m_difficulty };
	s_difficulty = difficulty;
	m_difficulty = arith_uint256(difficulty);

	if ((m_maxDifficulty != 0ull) && (m_difficulty != oldDifficulity))
	{
		onMessage(-1, "Info", "New difficulity detected (" + std::to_string(m_difficulty.GetLow64()) + ")...");
		updateTarget((m_maxDifficulty / m_difficulty).GetHex());
	}
}

void cpuSolver::setCustomDifficulty(uint32_t const customDifficulty)
{
	if (customDifficulty == 0u) return;

	s_customDifficulty = std::to_string(customDifficulty);
	m_customDifficulty = arith_uint256{ customDifficulty };

	onMessage(-1, "Info", "Custom difficulty (" + s_customDifficulty + ") detected...");
	updateTarget((m_maxDifficulty / m_customDifficulty).GetHex());
}

uint64_t cpuSolver::getTotalHashRate()
{
	using namespace std::chrono;
	uint64_t totalHashes{ 0ull };

	for (uint32_t id{ 0 }; id < m_miningThreadCount; ++id)
		totalHashes += m_threadHashes[id];

	return (uint64_t)((long double)totalHashes / (duration_cast<seconds>(steady_clock::now() - m_hashStartTime).count()));
}

uint64_t cpuSolver::getHashRateByThreadID(uint32_t const threadID)
{
	using namespace std::chrono;
	if (threadID < m_miningThreadCount)
		return (uint64_t)((long double)m_threadHashes[threadID] / (duration_cast<seconds>(steady_clock::now() - m_hashStartTime).count()));

	else return 0ull;
}

bool cpuSolver::islessThan(byte32_t &left, byte32_t &right)
{
	for (uint32_t i{ 0 }; i < UINT256_LENGTH; ++i)
	{
		if (left[i] < right[i]) return true;
		else if (left[i] > right[i]) return false;
	}
	return false;
}

void cpuSolver::startFinding()
{
	uint64_t lastPosition;
	resetWorkPosition(lastPosition);
	m_hashStartTime = std::chrono::steady_clock::now();

	for (uint32_t id{ 0 }; id < m_miningThreadCount; ++id)
	{
		std::thread t{ &cpuSolver::findSolution, this, id, m_miningThreadAffinities[id] };
		t.detach();
	}
}

void cpuSolver::stopFinding()
{
	for (uint32_t i{ 0 }; i < m_miningThreadCount; ++i) m_isThreadMining[i] = false;

	using namespace std::chrono_literals;
	std::this_thread::sleep_for(1s);
}

void cpuSolver::pauseFinding(bool pauseFinding)
{
	m_pause = pauseFinding;
}

// --------------------------------------------------------------------
// Private
// --------------------------------------------------------------------

bool cpuSolver::isAddressEmpty(address_t kingAddress)
{
	for (uint32_t i{ 0 }; i < ADDRESS_LENGTH; ++i)
		if (kingAddress[i] > 0u) return false;

	return true;
}

void cpuSolver::getKingAddress(address_t *kingAddress)
{
	m_getKingAddressCallback(kingAddress->data());
}

void cpuSolver::getSolutionTemplate(byte32_t *solutionTemplate)
{
	m_getSolutionTemplateCallback(solutionTemplate->data());
}

void cpuSolver::getWorkPosition(uint64_t &workPosition)
{
	m_getWorkPositionCallback(workPosition);
}

void cpuSolver::resetWorkPosition(uint64_t &lastPosition)
{
	m_resetWorkPositionCallback(lastPosition);
}

void cpuSolver::incrementWorkPosition(uint64_t &lastPosition, uint64_t increment)
{
	m_incrementWorkPositionCallback(lastPosition, increment);
}

void cpuSolver::onMessage(int threadID, const char* type, const char* message)
{
	m_messageCallback(threadID, type, message);
}

void cpuSolver::onMessage(int threadID, std::string type, std::string message)
{
	onMessage(threadID, type.c_str(), message.c_str());
}

void cpuSolver::onSolution(byte32_t const solution, byte32_t const digest, std::string challenge)
{
	if (!m_SubmitStale && challenge != s_challenge)
		return;
	else if (m_SubmitStale && challenge != s_challenge)
		onMessage(-1, "Warn", "Found stale solution, verifying...");
	else
		onMessage(-1, "Info", "Found solution, verifying...");

	if (!m_SubmitStale && challenge != s_challenge) return;

	std::string solutionStr{ bytesToHexString(solution) };

	std::string digestStr = bytesToHexString(digest);
	arith_uint256 arithDigest = arith_uint256(digestStr);
	onMessage(-1, "Debug", "Digest: 0x" + digestStr);

	if (arithDigest >= m_target)
	{
		onMessage(-1, "Error", "Verification failed: invalid solution"
			+ std::string("\nChallenge: ") + challenge
			+ "\nAddress: " + s_address
			+ "\nSolution: 0x" + solutionStr
			+ "\nDigest: 0x" + digestStr
			+ "\nTarget: " + s_target);
	}
	else
	{
		onMessage(-1, "Info", "Solution verified, submitting nonce 0x" + solutionStr + "...");
		m_solutionCallback(("0x" + digestStr).c_str(), s_address.c_str(), challenge.c_str(), s_difficulty.c_str(), s_target.c_str(), ("0x" + solutionStr).c_str(), m_customDifficulty > 0u);
	}
}

#	include <Windows.h>
bool cpuSolver::setCurrentThreadAffinity(uint32_t const affinityMask)
{
	return (bool)SetThreadAffinityMask(GetCurrentThread(), 1ull << affinityMask);
}

void cpuSolver::findSolution(uint32_t const threadID, uint32_t const affinityMask)
{
	try
	{
		byte32_t currentSolution;
		std::string currentChallenge;
		m_threadHashes[threadID] = 0ull;
		m_isThreadMining[threadID] = setCurrentThreadAffinity(affinityMask);

		if (m_isThreadMining[threadID]) onMessage(threadID, "Info", "Affinity mask to CPU " + std::to_string(affinityMask));
		else onMessage(threadID, "Error", "Failed to set affinity mask to CPU " + std::to_string(affinityMask));

		while (m_isThreadMining[threadID])
		{
			while (m_pause) { std::this_thread::sleep_for(std::chrono::milliseconds(500)); }

			if (currentChallenge != s_challenge)
			{
				char *c_currentChallenge = (char *)malloc(s_challenge.size());
				strcpy_s(c_currentChallenge, s_challenge.size() + 1, s_challenge.c_str());
				currentChallenge = std::string(c_currentChallenge);
			}

			getKingAddress(&m_kingAddress);
			getSolutionTemplate(&currentSolution);

			uint64_t nonce;
			incrementWorkPosition(nonce, 1ull);
			m_threadHashes[threadID]++;

			if (isAddressEmpty(m_kingAddress))
				std::memcpy(&currentSolution[12], &nonce, UINT64_LENGTH); // keep first and last 12 bytes, fill middle 8 bytes for mid state
			else
				std::memcpy(&currentSolution[ADDRESS_LENGTH], &nonce, UINT64_LENGTH); // Shifted for King address
			// no need to memcpy m_kingAddress as m_solutionTemplate already contains King address as prefix

			message_t miningMessage; // challenge32 + address20 + solution32
			std::memcpy(&miningMessage, &m_prefix, PREFIX_LENGTH); // challenge32 + address20
			std::memcpy(&miningMessage[PREFIX_LENGTH], &currentSolution, UINT256_LENGTH); // solution32

			byte32_t digest;
			keccak_256(&digest[0], UINT256_LENGTH, &miningMessage[0], MESSAGE_LENGTH);

			if (islessThan(digest, b_target))
				onSolution(currentSolution, digest, currentChallenge);

			if (nonce > INT64_MAX) resetWorkPosition(nonce);

			if (m_threadHashes[threadID] > INT32_MAX)
			{
				m_threadHashes[threadID] = 0ull;
				m_hashStartTime = std::chrono::steady_clock::now();
			}
		}
	}
	catch (std::exception &ex) { onMessage(affinityMask, "Error", ex.what()); }

	m_isThreadMining[threadID] = false;
	m_threadHashes[threadID] = 0ull;
}