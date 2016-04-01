-- premake5.lua
workspace "jchat"
	configurations { "Debug", "Release" }
	platforms { "Linux32", "Linux64", "OSX32", "OSX64", "Win32", "Win64" }

	project "jchat_server"
		kind "ConsoleApp"
		language "C++"
		targetdir "build/%{cfg.buildcfg}"

		includedirs { "jchat_lib/", "jchat_common/", "jchat_server/include/", "jchat_server/src/" }
		files { "jchat_lib/**", "jchat_common/**", "jchat_server/include/**", "jchat_server/src/**.cpp" }

		filter { "platforms:Linux32", "platforms:OSX32", "platforms:Win32" }
			architecture "x32"

		filter { "platforms:Linux64", "platforms:OSX64", "platforms:Win64" }
			architecture "x64"

		filter { "platforms:Linux32", "platforms:Linux64" }
			system "linux"

		filter { "platforms:OSX32", "platforms:OSX64" }
			system "macosx"

		filter { "platforms:Win32", "platforms:Win64" }
			system "windows"
			links { "ws2_32" }

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		configuration "Release"
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

		filter { "platforms:Linux32", "platforms:OSX32", "platforms:Win32" }
			architecture "x32"

		filter { "platforms:Linux64", "platforms:OSX64", "platforms:Win64" }
			architecture "x64"

		filter { "platforms:Linux32", "platforms:Linux64" }
			system "linux"

		filter { "platforms:OSX32", "platforms:OSX64" }
			system "macosx"

		filter { "platforms:Win32", "platforms:Win64" }
			system "windows"
			links { "ws2_32" }

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		configuration "Release"
			defines { "NDEBUG" }
			optimize "On"

		configuration { "gmake" }
			buildoptions { "-std=c++11" }
			linkoptions { "-pthread" }
