#include "BlasterLogging.h"

// The implementation of log category top use within application
DEFINE_LOG_CATEGORY(Blaster);

FString BlasterGetClientServerContextString(const UObject* Object)
{
    if (!Object)
    {
        return TEXT("?");
    }
    else
    {
        FString Context{ TEXT("") };
        const auto World = Object->GetWorld();
        if (EWorldType::Game == World->WorldType)
        {
            Context.Append("Game");
        }
        else if (EWorldType::PIE == World->WorldType)
        {
            Context.Append("PIE");
        }
        else if (EWorldType::Editor == World->WorldType)
        {
            Context.Append("Editor");
        }
        else if (EWorldType::EditorPreview == World->WorldType)
        {
            Context.Append("EditorPreview");
        }
        else if (EWorldType::GamePreview == World->WorldType)
        {
            Context.Append("GamePreview");
        }
        else if (EWorldType::GameRPC == World->WorldType)
        {
            Context.Append("GameRPC");
        }
        else if (World->IsPlayInPreview())
        {
            Context.Append("Preview");
        }
        // ReSharper disable once CppTooWideScopeInitStatement
        const ENetMode NetMode = World->GetNetMode();
        if (NM_Client == NetMode)
        {
            Context.Append(":Client");
        }
        else if (NM_DedicatedServer == NetMode || NM_ListenServer == NetMode)
        {
            Context.Append(":Server");
        }

        return Context;
    }
}
