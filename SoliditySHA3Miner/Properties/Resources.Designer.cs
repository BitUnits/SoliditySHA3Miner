﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.42000
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace SoliditySHA3Miner.Properties {
    using System;
    
    
    /// <summary>
    ///   A strongly-typed resource class, for looking up localized strings, etc.
    /// </summary>
    // This class was auto-generated by the StronglyTypedResourceBuilder
    // class via a tool like ResGen or Visual Studio.
    // To add or remove a member, edit your .ResX file then rerun ResGen
    // with the /str option, or rebuild your VS project.
    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("System.Resources.Tools.StronglyTypedResourceBuilder", "15.0.0.0")]
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
    [global::System.Runtime.CompilerServices.CompilerGeneratedAttribute()]
    internal class Resources {
        
        private static global::System.Resources.ResourceManager resourceMan;
        
        private static global::System.Globalization.CultureInfo resourceCulture;
        
        [global::System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
        internal Resources() {
        }
        
        /// <summary>
        ///   Returns the cached ResourceManager instance used by this class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Resources.ResourceManager ResourceManager {
            get {
                if (object.ReferenceEquals(resourceMan, null)) {
                    global::System.Resources.ResourceManager temp = new global::System.Resources.ResourceManager("SoliditySHA3Miner.Properties.Resources", typeof(Resources).Assembly);
                    resourceMan = temp;
                }
                return resourceMan;
            }
        }
        
        /// <summary>
        ///   Overrides the current thread's CurrentUICulture property for all
        ///   resource lookups using this strongly typed resource class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Globalization.CultureInfo Culture {
            get {
                return resourceCulture;
            }
            set {
                resourceCulture = value;
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to #define OPENCL_PLATFORM_UNKNOWN	0
        ///#define OPENCL_PLATFORM_NVIDIA	1
        ///#define OPENCL_PLATFORM_AMD		2
        ///
        ///#ifndef PLATFORM
        ///#	define PLATFORM				OPENCL_PLATFORM_UNKNOWN
        ///#endif
        ///
        ///#if PLATFORM == OPENCL_PLATFORM_AMD
        ///#	pragma OPENCL EXTENSION		cl_amd_media_ops : enable
        ///#endif
        ///
        ///#ifndef COMPUTE
        ///#	define COMPUTE				0
        ///#endif
        ///
        ///#define MAX_SOLUTION_COUNT		32u
        ///#define STATE_LENGTH			200u
        ///
        ///typedef union _nonce_t
        ///{
        ///	uint2		uint2_s;
        ///	ulong		ulong_s;
        ///} nonce_t;
        ///
        ///typedef union _state_t
        ///{
        ///	uint2		uint2_s[ [rest of string was truncated]&quot;;.
        /// </summary>
        internal static string sha3Kernel {
            get {
                return ResourceManager.GetString("sha3Kernel", resourceCulture);
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to #define OPENCL_PLATFORM_UNKNOWN	0
        ///#define OPENCL_PLATFORM_NVIDIA	1
        ///#define OPENCL_PLATFORM_AMD		2
        ///
        ///#ifndef PLATFORM
        ///#define PLATFORM				OPENCL_PLATFORM_UNKNOWN
        ///#endif
        ///
        ///#if PLATFORM == OPENCL_PLATFORM_AMD
        ///#pragma OPENCL EXTENSION		cl_amd_media_ops : enable
        ///#endif
        ///
        ///#ifndef COMPUTE
        ///#define COMPUTE					0
        ///#endif
        ///
        ///#define MAX_SOLUTION_COUNT		32u
        ///#define ADDRESS_LENGTH			20u
        ///#define UINT64_LENGTH			8u
        ///#define UINT256_LENGTH			32u
        ///#define MESSAGE_LENGTH			84u
        ///#define SPONGE_LENGTH			200u
        ///#defi [rest of string was truncated]&quot;;.
        /// </summary>
        internal static string sha3KingKernel {
            get {
                return ResourceManager.GetString("sha3KingKernel", resourceCulture);
            }
        }
    }
}
