#include "AICommunicator.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Async/Async.h"

AAICommunicator::AAICommunicator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AAICommunicator::SendPromptToAI(const FString& Prompt, const FString& UserID)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(TEXT("http://127.0.0.1:18789/v1/chat/completions")); 
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ..."));

    // สร้าง JSON Body
    TSharedPtr<FJsonObject> RootJsonObj = MakeShareable(new FJsonObject());
    RootJsonObj->SetStringField(TEXT("model"), TEXT("default"));
    
    // 🔑 ใช้ค่า UserID ที่ส่งมาจาก Blueprint (ไม่ต้องใส่ TEXT() ครอบตัวแปร)
    RootJsonObj->SetStringField(TEXT("user"), UserID);

    TArray<TSharedPtr<FJsonValue>> MessagesArray;
    TSharedPtr<FJsonObject> MessageObj = MakeShareable(new FJsonObject());
    MessageObj->SetStringField(TEXT("role"), TEXT("user"));
    MessageObj->SetStringField(TEXT("content"), Prompt);
    MessagesArray.Add(MakeShareable(new FJsonValueObject(MessageObj)));

    RootJsonObj->SetArrayField(TEXT("messages"), MessagesArray);

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootJsonObj.ToSharedRef(), Writer);

    Request->SetContentAsString(JsonString);
    Request->OnProcessRequestComplete().BindUObject(this, &AAICommunicator::OnHttpResponseReceived);
    Request->ProcessRequest();
}

void AAICommunicator::OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
            if (JsonObject->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray->Num() > 0)
            {
                TSharedPtr<FJsonObject> FirstChoice = (*ChoicesArray)[0]->AsObject();
                const TSharedPtr<FJsonObject>* MessageObj;

                if (FirstChoice->TryGetObjectField(TEXT("message"), MessageObj))
                {
                    FString ReplyText;
                    if ((*MessageObj)->TryGetStringField(TEXT("content"), ReplyText))
                    {
                        AsyncTask(ENamedThreads::GameThread, [this, ReplyText]()
                        {
                            OnResponseReceived.Broadcast(ReplyText);
                        });
                        UE_LOG(LogTemp, Log, TEXT("AI_LOG: %s"), *ReplyText);
                    }
                }
            }
        }
    }
}