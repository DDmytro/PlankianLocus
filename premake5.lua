solution "PlankianLocus"

language "C++"
configurations { "Debug", "Release" }

configuration "Debug"
    flags {"Symbols"}
configuration {}

-- find OpenCL
if os.is("windows") or os.is("linux") then
        local openclpath = os.getenv("AMDAPPSDKROOT")
        if not openclpath then
           openclpath = os.getenv("CUDA_PATH")
        end
        if not openclpath then
           openclpath = os.getenv("INTELOCLSDKROOT")
        end
        if not openclpath and os.is("linux") then
           libdirs {"/usr/lib/x86_64-linux-gnu"}
           libdirs {"/usr/include"}
           links {"OpenCL"}
        end
        
        if(openclpath) then
           subpath = string.gsub(openclpath, "\\", "/");
           includedirs{string.format("%s/include",subpath)};
           libdirs{string.format("%s/lib/x86_64",subpath)};
           links {"OpenCL"}
        end
end       
    
if os.is("macosx") then
   linkoptions{ "-framework OpenCL" }
   buildoptions{ "-std=c++11 -stdlib=libc++"}
else if os.is("linux") then
   buildoptions "-std=c++11"
   links {"pthread"}
end
end

project "PlankianLocus"
location "./src"
files { "**.cpp", "**.h"}
kind "ConsoleApp"
targetdir "."

