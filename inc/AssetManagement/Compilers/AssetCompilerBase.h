#ifndef ASSET_COMPILER_BASE_H
#define ASSET_COMPILER_BASE_H

/*****************************************************
    Includes
*****************************************************/
#include <string>

class AssetCompilerBase
{
public:
    virtual void Compile(std::string const& _filePath) = 0;
    void RunExecutable(std::string& _executableCmd);
};

#endif