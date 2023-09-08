// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
{
    if (const auto Subsystem = IOnlineSubsystem::Get())
    {
        OnlineSessionInterface = Subsystem->GetSessionInterface();
    }
    else
    {
        OnlineSessionInterface = nullptr;
    }
}
