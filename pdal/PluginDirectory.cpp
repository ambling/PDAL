/******************************************************************************
* Copyright (c) 2018, Hobu Inc. (info@hobu.co)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include <pdal/PluginDirectory.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/pdal_config.hpp>

namespace pdal
{

namespace
{

#if defined(__APPLE__) && defined(__MACH__)
    const std::string dynamicLibraryExtension("dylib");
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__) || defined(__GNU__)
    const std::string dynamicLibraryExtension("so");
#elif defined _WIN32
    const std::string dynamicLibraryExtension("dll");
#endif

StringList pluginSearchPaths()
{
    StringList searchPaths;
    std::string envOverride;

    Utils::getenv("PDAL_DRIVER_PATH", envOverride);

    if (!envOverride.empty())
        searchPaths = Utils::split2(envOverride, ':');
    else
    {
        StringList standardPaths { ".", "./lib", "../lib", "./bin", "../bin" };
        for (std::string& s : standardPaths)
        {
            if (FileUtils::toAbsolutePath(s) !=
                FileUtils::toAbsolutePath(Config::pluginInstallPath()))
                searchPaths.push_back(s);
        }
        searchPaths.push_back(Config::pluginInstallPath());
    }
    return searchPaths;
}

// libpdal_plugin_{stagetype}_{name}.{extension}
// For example, libpdal_plugin_writer_text.so or
// libpdal_plugin_filter_color.dylib
std::string validPlugin(const std::string& path, const StringList& types)
{
    auto typeValid = [&types](std::string& type)
    {
        for (auto t: types)
            if (type == t)
                return true;
        return false;
    };

    std::string file = FileUtils::getFilename(path);
    StringList parts = Utils::split(file, '_');
    if (parts.size() != 4 || parts[0] != "libpdal" || parts[1] != "plugin")
        return std::string();
    StringList subparts = Utils::split(parts[3], '.');

    if (subparts.size() != 2 || subparts[1] != dynamicLibraryExtension)
        return std::string();

    std::string type = parts[2];
    std::string plugin = subparts[0];

    if (!typeValid(type))
        return std::string();
    return type + "s." + plugin;
}

} // unnamed namespace;


PluginDirectory::PluginDirectory()
{
    for (const auto& dir : pluginSearchPaths())
    {
        StringList files = FileUtils::directoryList(dir);
        for (auto& file : files)
        {
            file = FileUtils::toAbsolutePath(file);

            std::string plugin;
            plugin = validPlugin(file, {"kernel"});
            if (plugin.size())
            {
                m_kernels.insert(std::make_pair(plugin, file));
                continue;
            }
            plugin = validPlugin(file, {"reader", "writer", "filter"});
            if (plugin.size())
                m_drivers.insert(std::make_pair(plugin, file));
        }
    }
}

StringList PluginDirectory::test_pluginSearchPaths()
{
    return pluginSearchPaths();
}

} // namespace pdal

