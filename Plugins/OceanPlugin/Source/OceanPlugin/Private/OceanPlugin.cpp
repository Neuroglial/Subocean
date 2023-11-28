// Copyright Epic Games, Inc. All Rights Reserved.

#include "OceanPlugin.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FOceanPluginModule"

void FOceanPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	auto shaderPath = IPluginManager::Get().FindPlugin(TEXT("OceanPlugin"))->GetBaseDir() + TEXT("/Source/Shaders");
	AddShaderSourceDirectoryMapping(TEXT("/Shaders"), shaderPath);

}

void FOceanPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOceanPluginModule, OceanPlugin)