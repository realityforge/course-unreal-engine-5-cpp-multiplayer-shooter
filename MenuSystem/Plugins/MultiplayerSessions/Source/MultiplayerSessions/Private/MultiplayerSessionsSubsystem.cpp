// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
{
    const auto Subsystem = IOnlineSubsystem::Get();
    OnlineSessionInterface = Subsystem ? Subsystem->GetSessionInterface() : nullptr;
}
