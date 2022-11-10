// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ThirdPersonMPServerTarget : TargetRules
{
    public ThirdPersonMPServerTarget(TargetInfo Target) : base(Target) //根据你的项目名称更改此行
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.Add("ThirdPersonMP"); //根据你的项目名称更改此行
    }
}
