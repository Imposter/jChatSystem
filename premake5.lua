-- premake5.lua
workspace "jchat"
	configurations {
		"Linux32_Debug",
		"Linux64_Debug",
		"OSX32_Debug",
		"OSX64_Debug",
		"Win32_Debug",
		"Win64_Debug",
		"Linux32_Release",
		"Linux64_Release",
		"OSX32_Release",
		"OSX64_Release",
		"Win32_Release",
		"Win64_Release"
	}

	project "jchat_server"
		kind "ConsoleApp"
		language "C++"
		targetdir "build/%{cfg.buildcfg}"

		includedirs { "jchat_lib/", "jchat_common/", "jchat_server/include/", "jchat_server/src/" }
		files { "jchat_lib/**", "jchat_common/**", "jchat_server/include/**", "jchat_server/src/**.cpp" }

		configuration { "Linux32_*", "OSX32_*", "Win32_*" }
			architecture "x32"

		configuration { "Linux64_*", "OSX64_*", "Win64_*" }
			architecture "x64"

		configuration { "Linux32_*", "Linux64_*" }
			system "linux"

		configuration { "OSX32_*", "OSX64_*" }
			system "macosx"

		configuration { "Win32_*", "Win64_*" }
			system "windows"
			links { "ws2_32" }

		configuration "*_Debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		configuration "*_Release"
			defines { "NDEBUG" }
			optimize "On"

		configuration { "gmake" }
			buildoptions { "-std=c++11" }
			linkoptions { "-pthread" }

	project "jchat_client"
		kind "ConsoleApp"
		language "C++"
		targetdir "build/%{cfg.buildcfg}"

		includedirs { "jchat_lib/", "jchat_common/", "jchat_client/include/", "jchat_client/src/" }
		files { "jchat_lib/**", "jchat_common/**", "jchat_client/include/**", "jchat_client/src/**.cpp" }

		configuration { "Linux32_*", "OSX32_*", "Win32_*" }
			architecture "x32"

		configuration { "Linux64_*", "OSX64_*", "Win64_*" }
			architecture "x64"

		configuration { "Linux32_*", "Linux64_*" }
			system "linux"

		configuration { "OSX32_*", "OSX64_*" }
			system "macosx"

		configuration { "Win32_*", "Win64_*" }
			system "windows"
			links { "ws2_32" }

		configuration "*_Debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		configuration "*_Release"
			defines { "NDEBUG" }
			optimize "On"
