#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IHttpRequest.h"
#include "AICommunicator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResponseReceived, const FString&, ResponseText);

UCLASS()
class MYPROJECT_API AAICommunicator : public AActor
{
    GENERATED_BODY()
    
public:    
    AAICommunicator();

    // เพิ่ม Parameter ชื่อ User เข้ามาเพื่อให้ Blueprint กำหนด Session ID ได้เอง
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SendPromptToAI(const FString& Prompt, const FString& UserID);

    UPROPERTY(BlueprintAssignable, Category = "AI")
    FOnResponseReceived OnResponseReceived;

private:
    void OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};