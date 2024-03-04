#include "CustomLogger.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

void CustomLogger::Print(const FString& Message, const FColor& Color)
{
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, *Message);
	}
}

void CustomLogger::PrintLog(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
}

EAppReturnType::Type CustomLogger::ShowMessageDialog(EAppMsgType::Type MessageType, const FString& Message, bool bShowMessageAsWarning)
{
	if(bShowMessageAsWarning)
	{
		const FText MessageTitle = FText::FromString(TEXT("Warning"));

		return FMessageDialog::Open(MessageType, FText::FromString(*Message), MessageTitle);
	}
	else
	{
		return FMessageDialog::Open(MessageType, FText::FromString(*Message));
	}
}

void CustomLogger::ShowNotifyInfo(const FString& Message)
{
	FNotificationInfo NotificationInfo(FText::FromString(Message));
	NotificationInfo.bUseLargeFont = true;
	NotificationInfo.FadeOutDuration = 7.f;

	FSlateNotificationManager::Get().AddNotification(NotificationInfo);
}
