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

// ply instantiate Test
void inst_Test(TargetInstantiatorArgs* args) {
    args->buildTarget->targetType = BuildTargetType::EXE;
    args->addSourceFiles("src/test", false);
    args->addExtern(Visibility::Public, "liburho3d");
    args->addResourceDir("src/test/CoreData","CoreData");
    args->addResourceDir("src/test/Data","Data");
}




// ply extern urho3d.liburho3d.prebuilt
ExternResult extern_urho3d_prebuilt(ExternCommand cmd, ExternProviderArgs* args) {
    // Toolchain filters

    StringView version = "1.8-ALPHA.94";
    StringView platform = "Linux";
    StringView ext = ".tar.gz";

    String plyPlatform = args->toolchain->targetPlatform.name;
    if (plyPlatform == "windows"){
        platform = "Windows";
        ext =".zip";
    }
    else if (plyPlatform == "macos"){
        platform = "macOS";
    }
    else if (plyPlatform != "linux"){
        return {ExternResult::UnsupportedToolchain, String::format("Unsupported Platform:{}",plyPlatform)};
    }

    String filenameWithoutExt = String::format("Urho3D-{}-{}-64bit-STATIC-snapshot",version,platform);
    String filename = String::format("{}{}",filenameWithoutExt,ext);
    String url = String::format("https://sourceforge.net/projects/urho3d/files/Urho3D/Snapshots/{}/download",filename);
//    String url = String::format("http://localhost/web/{}",filename);


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
        args->dep->libs.append(NativePath::join(installFolder, "lib/Urho3D/libUrho3D.a"));
        return {ExternResult::Instantiated, ""};
    }
    PLY_ASSERT(0);
    return {ExternResult::Unknown, ""};
}
