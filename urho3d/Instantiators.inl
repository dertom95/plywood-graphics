#ifndef generating
 #include<ply-build-repo/TargetInstantiatorArgs.h>
 #include<ply-runtime/string/String.h>
 #include<ply-build-repo/ProjectInstantiator.h>
 #include<ply-build-repo/ProjectInstantiationEnv.h>
 #include<ply-build-provider/HostTools.h>
 #include<ply-build-provider/PackageManager.h>
 #include<ply-build-provider/ExternFolderRegistry.h>
 #include<ply-build-provider/ExternHelpers.h>
 #include<ply-runtime/algorithm/Find.h>

 using namespace ply::build;
 using namespace ply;
#endif

#include <iostream>

// ply instantiate Urho3DTestScene
void inst_Urho3DTestScene(TargetInstantiatorArgs* args) {
    args->buildTarget->targetType = BuildTargetType::EXE;
    args->addSourceFiles("src/test", false);
    args->addExtern(Visibility::Public, "liburho3d");
    args->addResourceDir("src/test/CoreData","CoreData");
    args->addResourceDir("src/test/Data","Data");
}




// ply extern plywood-graphics.liburho3d.prebuilt
ExternResult extern_urho3d_prebuilt(ExternCommand cmd, ExternProviderArgs* args) {
    // Toolchain filters

    StringView version = "1.8-ALPHA.94";
    StringView platform = "";
    StringView ext = ".tar.gz";
    StringView staticLibExt = ".a";
    StringView libPrefix = "lib";

    String plyPlatform = args->toolchain->targetPlatform.name;
    if (plyPlatform == "windows"){
        platform = "Windows-64bit-STATIC-3D11";
        ext =".zip";
        staticLibExt=".lib";
        libPrefix="";
    }
    else if (plyPlatform == "macos"){
        platform = "macOS-64bit-STATIC";
    }
    else if (plyPlatform == "linux"){
        platform = "Linux-64bit-STATIC";
    }
    else {
        return {ExternResult::UnsupportedToolchain, String::format("Unsupported Platform:{}",plyPlatform)};
    }
    String filenameWithoutExt = String::format("Urho3D-{}-{}-snapshot",version,platform);
    String filename = String::format("{}{}",filenameWithoutExt,ext);
    String url = String::format("https://sourceforge.net/projects/urho3d/files/Urho3D/Snapshots/{}/download",filename);

    // Handle Command
    Tuple<ExternResult, ExternFolder*> er = args->findExistingExternFolder(plyPlatform);
    if (cmd == ExternCommand::Status) {
        return er.first;
    } else if (cmd == ExternCommand::Install) {
        std::cout << "install" <<std::endl;
        if (er.first.code != ExternResult::SupportedButNotInstalled) {
            return er.first;
        }
        ExternFolder* externFolder = args->createExternFolder(plyPlatform);

        String archivePath = NativePath::join(externFolder->path, filename);

        if (!downloadFile(archivePath, url)) {
            return {ExternResult::InstallFailed, String::format("Error downloading '{}'", url)};
        }
        if (!extractFile(archivePath)) {
            return {ExternResult::InstallFailed,
                    String::format("Error extracting '{}'", archivePath)};
        }
        FileSystem::native()->deleteFile(archivePath);
        externFolder->success = true;
        externFolder->save();
        return {ExternResult::Installed, ""};
    } else if (cmd == ExternCommand::Instantiate) {
        if (er.first.code != ExternResult::Installed) {
            return er.first;
        }
        String installFolder = NativePath::join(er.second->path, filenameWithoutExt);
        args->dep->includeDirs.append(NativePath::join(installFolder, "include"));
        args->dep->includeDirs.append(NativePath::join(installFolder, "include/Urho3D/ThirdParty"));
        args->dep->libs.append("dl");
        args->dep->libs.append("GL");
        args->dep->libs.append(NativePath::join(installFolder, String::format("lib/Urho3D/{}Urho3D{}",libPrefix,staticLibExt) ));
        return {ExternResult::Instantiated, ""};
    }
    PLY_ASSERT(0);
    return {ExternResult::Unknown, ""};
}
