//
// Created by stephane bourque on 2021-06-22.
//

#include "RESTAPI_action_links.h"
#include "StorageService.h"
#include "Utils.h"
#include "RESTAPI_utils.h"

#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTMLForm.h"
#include "RESTAPI_server.h"
#include "Daemon.h"

namespace OpenWifi {
    void RESTAPI_action_links::handleRequest(Poco::Net::HTTPServerRequest &Request,
                       Poco::Net::HTTPServerResponse &Response) {
        //  there is no authentication here, this is just someone clicking on a link
        //  and arriving here. There should be a UUID in the link and this is all we need to know
        //  what we need to do.
        ParseParameters(Request);

        auto Action = GetParameter("action","");
        auto Id = GetParameter("id","");

        if(Action=="password_reset")
            DoResetPassword(Id, Request, Response);
        else if(Action=="email_verification")
            DoEmailVerification(Id, Request, Response);
        else
            DoReturnA404(Request, Response);
    }

    void RESTAPI_action_links::DoResetPassword(std::string &Id,Poco::Net::HTTPServerRequest &Request,
                         Poco::Net::HTTPServerResponse &Response) {

        if(Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_GET) {
            Poco::File  FormFile{ RESTAPI_Server()->AssetDir() + "/password_reset.html"};
            Types::StringPairVec    FormVars{ {"UUID", Id},
                                              {"PASSWORD_VALIDATION", AuthService()->PasswordValidationExpression()}};
            SendHTMLFileBack(FormFile,Request, Response, FormVars);
        } else if(Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_POST) {
            //  form has been posted...
            RESTAPI_PartHandler PartHandler;
            Poco::Net::HTMLForm Form(Request, Request.stream(), PartHandler);
            if (!Form.empty()) {
                auto Password1 = Form.get("password1","bla");
                auto Password2 = Form.get("password1","blu");
                Id = Form.get("id","");
                if(Password1!=Password2 || !AuthService()->ValidatePassword(Password2) || !AuthService()->ValidatePassword(Password1)) {
                    Poco::File  FormFile{ RESTAPI_Server()->AssetDir() + "/password_reset_error.html"};
                    Types::StringPairVec    FormVars{ {"UUID", Id},
                                                      {"ERROR_TEXT", "For some reason, the passwords entered do not match or they do not comply with"
                                                                     " accepted password creation restrictions. Please consult our on-line help"
                                                                     " to look at the our password policy. If you would like to contact us, please mention"
                                                                     " id(" + Id + ")"}};
                    SendHTMLFileBack(FormFile,Request, Response, FormVars);
                    return;
                }

                SecurityObjects::UserInfo   UInfo;
                if(!Storage()->GetUserById(Id,UInfo)) {
                    Poco::File  FormFile{ RESTAPI_Server()->AssetDir() + "/password_reset_error.html"};
                    Types::StringPairVec    FormVars{ {"UUID", Id},
                                                      {"ERROR_TEXT", "This request does not contain a valid user ID. Please contact your system administrator."}};
                    SendHTMLFileBack(FormFile,Request, Response, FormVars);
                    return;
                }

                if(UInfo.blackListed || UInfo.suspended) {
                    Poco::File  FormFile{ RESTAPI_Server()->AssetDir() + "/password_reset_error.html"};
                    Types::StringPairVec    FormVars{ {"UUID", Id},
                                                      {"ERROR_TEXT", "Please contact our system administrators. We have identified an error in your account that must be resolved first."}};
                    SendHTMLFileBack(FormFile,Request, Response, FormVars);
                    return;
                }

                if(!AuthService()->SetPassword(Password1,UInfo)) {
                    Poco::File  FormFile{ RESTAPI_Server()->AssetDir() + "/password_reset_error.html"};
                    Types::StringPairVec    FormVars{ {"UUID", Id},
                                                      {"ERROR_TEXT", "You cannot reuse one of your recent passwords."}};
                    SendHTMLFileBack(FormFile,Request, Response, FormVars);
                    return;
                }
                Storage()->UpdateUserInfo(UInfo.email,Id,UInfo);
                Poco::File  FormFile{ RESTAPI_Server()->AssetDir() + "/password_reset_success.html"};
                Types::StringPairVec    FormVars{ {"UUID", Id},
                                                  {"USERNAME", UInfo.email},
                                                  {"ACTION_LINK",Daemon()->GetUIURI()}};
                SendHTMLFileBack(FormFile,Request, Response, FormVars);
            }
        } else {
            DoReturnA404(Request, Response);
        }
    }

    void RESTAPI_action_links::DoEmailVerification(std::string &Id,Poco::Net::HTTPServerRequest &Request,
                             Poco::Net::HTTPServerResponse &Response) {
        if(Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_GET) {
            SecurityObjects::UserInfo UInfo;

            if (!Storage()->GetUserById(Id, UInfo)) {
                Types::StringPairVec FormVars{{"UUID",       Id},
                                              {"ERROR_TEXT", "This does not appear to be a valid email verification link.."}};
                Poco::File FormFile{RESTAPI_Server()->AssetDir() + "/email_verification_error.html"};
                SendHTMLFileBack(FormFile, Request, Response, FormVars);
                return;
            }

            UInfo.waitingForEmailCheck = false;
            UInfo.validated = true;
            UInfo.lastEmailCheck = std::time(nullptr);
            UInfo.validationDate = std::time(nullptr);
            Storage()->UpdateUserInfo(UInfo.email, Id, UInfo);
            Types::StringPairVec FormVars{{"UUID",     Id},
                                          {"USERNAME", UInfo.email},
                                          {"ACTION_LINK",Daemon()->GetUIURI()}};
            Poco::File FormFile{RESTAPI_Server()->AssetDir() + "/email_verification_success.html"};
            SendHTMLFileBack(FormFile, Request, Response, FormVars);
            return;
        } else {
            DoReturnA404(Request, Response);
        }
    }

    void RESTAPI_action_links::DoReturnA404(Poco::Net::HTTPServerRequest &Request,
                                            Poco::Net::HTTPServerResponse &Response) {
        Types::StringPairVec FormVars;
        Poco::File FormFile{RESTAPI_Server()->AssetDir() + "/404_error.html"};
        SendHTMLFileBack(FormFile, Request, Response, FormVars);
    }

}
