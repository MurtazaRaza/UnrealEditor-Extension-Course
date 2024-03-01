#pragma once

class DebugHeader
{
public:

	static void Print(const FString& Message, const FColor& Color);
	static void PrintLog(const FString& Message);
	static EAppReturnType::Type ShowMessageDialog(EAppMsgType::Type MessageType, const FString& Message, bool bShowMessageAsWarning = true);
	static void ShowNotifyInfo(const FString& Message);
	
};
