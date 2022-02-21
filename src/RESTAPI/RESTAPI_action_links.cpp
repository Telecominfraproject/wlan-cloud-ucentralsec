//
// Created by stephane bourque on 2021-06-22.
//

#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTMLForm.h"

#include "RESTAPI_action_links.h"
#include "StorageService.h"
#include "framework/MicroService.h"
#include "Daemon.h"

namespace OpenWifi {

    void RESTAPI_action_links::DoGet() {

        auto Action = GetParameter("action","");
        auto Id = GetParameter("id","");

        SecurityObjects::ActionLink Link;
        if(!StorageService()->ActionLinksDB().GetActionLink(Id,Link))
            return DoReturnA404();

        if(Action=="password_reset")
            return RequestResetPassword(Link);
        else if(Action=="email_verification")
            return DoEmailVerification(Link);
        else if(Action=="signup_verification")
            return DoNewSubVerification(Link);
        else
            return DoReturnA404();
    }

    void RESTAPI_action_links::DoPost() {
        auto Action = GetParameter("action","");

        if(Action=="password_reset")
            return CompleteResetPassword();
        else if(Action=="signup_completion")
            return CompleteSubVerification();
        else
            return DoReturnA404();
    }

    void RESTAPI_action_links::RequestResetPassword(SecurityObjects::ActionLink &Link) {
        Logger_.information(Poco::format("REQUEST-PASSWORD-RESET(%s): For ID=%s", Request->clientAddress().toString(), Link.userId));
        Poco::File  FormFile{ Daemon()->AssetDir() + "/password_reset.html"};
        Types::StringPairVec    FormVars{ {"UUID", Link.id},
                                          {"PASSWORD_VALIDATION", AuthService()->PasswordValidationExpression()}};
        SendHTMLFileBack(FormFile,FormVars);
    }

    void RESTAPI_action_links::DoNewSubVerification(SecurityObjects::ActionLink &Link) {
        Logger_.information(Poco::format("REQUEST-SUB-SIGNUP(%s): For ID=%s", Request->clientAddress().toString(), Link.userId));
        Poco::File  FormFile{ Daemon()->AssetDir() + "/signup_verification.html"};
        Types::StringPairVec    FormVars{ {"UUID", Link.id},
                                          {"PASSWORD_VALIDATION", AuthService()->PasswordValidationExpression()}};
        SendHTMLFileBack(FormFile,FormVars);
    }

    void RESTAPI_action_links::CompleteResetPassword() {
        //  form has been posted...
        RESTAPI_PartHandler PartHandler;
        Poco::Net::HTMLForm Form(*Request, Request->stream(), PartHandler);
        if (!Form.empty()) {

            auto Password1 = Form.get("password1","bla");
            auto Password2 = Form.get("password2","blu");
            auto Id = Form.get("id","");
            auto Now = std::time(nullptr);

            SecurityObjects::ActionLink Link;
            if(!StorageService()->ActionLinksDB().GetActionLink(Id,Link))
                return DoReturnA404();

            if(Now > Link.expires) {
                StorageService()->ActionLinksDB().CancelAction(Id);
                return DoReturnA404();
            }

            if(Password1!=Password2 || !AuthService()->ValidatePassword(Password2) || !AuthService()->ValidatePassword(Password1)) {
                Poco::File  FormFile{ Daemon()->AssetDir() + "/password_reset_error.html"};
                Types::StringPairVec    FormVars{ {"UUID", Id},
                                                  {"ERROR_TEXT", "For some reason, the passwords entered do not match or they do not comply with"
                                                                 " accepted password creation restrictions. Please consult our on-line help"
                                                                 " to look at the our password policy. If you would like to contact us, please mention"
                                                                 " id(" + Id + ")"}};
                return SendHTMLFileBack(FormFile,FormVars);
            }

            SecurityObjects::UserInfo   UInfo;

            bool Found = Link.userAction ? StorageService()->UserDB().GetUserById(Link.userId,UInfo) : StorageService()->SubDB().GetUserById(Link.userId,UInfo);
            if(!Found) {
                Poco::File  FormFile{ Daemon()->AssetDir() + "/password_reset_error.html"};
                Types::StringPairVec    FormVars{ {"UUID", Id},
                                                  {"ERROR_TEXT", "This request does not contain a valid user ID. Please contact your system administrator."}};
                return SendHTMLFileBack(FormFile,FormVars);
            }

            if(UInfo.blackListed || UInfo.suspended) {
                Poco::File  FormFile{ Daemon()->AssetDir() + "/password_reset_error.html"};
                Types::StringPairVec    FormVars{ {"UUID", Id},
                                                  {"ERROR_TEXT", "Please contact our system administrators. We have identified an error in your account that must be resolved first."}};
                return SendHTMLFileBack(FormFile,FormVars);
            }

            bool GoodPassword = Link.userAction ? AuthService()->SetPassword(Password1,UInfo) : AuthService()->SetSubPassword(Password1,UInfo);
            if(!GoodPassword) {
                Poco::File  FormFile{ Daemon()->AssetDir() + "/password_reset_error.html"};
                Types::StringPairVec    FormVars{ {"UUID", Id},
                                                  {"ERROR_TEXT", "You cannot reuse one of your recent passwords."}};
                return SendHTMLFileBack(FormFile,FormVars);
            }

            UInfo.modified = std::time(nullptr);
            if(Link.userAction)
                StorageService()->UserDB().UpdateUserInfo(UInfo.email,Link.userId,UInfo);
            else
                StorageService()->SubDB().UpdateUserInfo(UInfo.email,Link.userId,UInfo);

            Poco::File  FormFile{ Daemon()->AssetDir() + "/password_reset_success.html"};
            Types::StringPairVec    FormVars{ {"UUID", Id},
                                              {"USERNAME", UInfo.email},
                                              {"ACTION_LINK",MicroService::instance().GetUIURI()}};
            StorageService()->ActionLinksDB().CompleteAction(Id);
            SendHTMLFileBack(FormFile,FormVars);
        } else {
            DoReturnA404();
        }
    }

    void RESTAPI_action_links::CompleteSubVerification() {
        RESTAPI_PartHandler PartHandler;
        Poco::Net::HTMLForm Form(*Request, Request->stream(), PartHandler);

        if (!Form.empty()) {
            auto Password1 = Form.get("password1","bla");
            auto Password2 = Form.get("password2","blu");
            auto Id = Form.get("id","");
            auto Now = std::time(nullptr);

            SecurityObjects::ActionLink Link;
            if(!StorageService()->ActionLinksDB().GetActionLink(Id,Link)) {
                return DoReturnA404();
            }

            if(Now > Link.expires) {
                StorageService()->ActionLinksDB().CancelAction(Id);
                return DoReturnA404();
            }

            if(Password1!=Password2 || !AuthService()->ValidateSubPassword(Password1)) {
                Poco::File  FormFile{ Daemon()->AssetDir() + "/password_reset_error.html"};
                Types::StringPairVec    FormVars{ {"UUID", Id},
                                                  {"ERROR_TEXT", "For some reason, the passwords entered do not match or they do not comply with"
                                                                 " accepted password creation restrictions. Please consult our on-line help"
                                                                 " to look at the our password policy. If you would like to contact us, please mention"
                                                                 " id(" + Id + ")"}};
                return SendHTMLFileBack(FormFile,FormVars);
            }

            SecurityObjects::UserInfo   UInfo;
            bool Found = StorageService()->SubDB().GetUserById(Link.userId,UInfo);
            if(!Found) {
                Poco::File  FormFile{ Daemon()->AssetDir() + "/signup_verification_error.html"};
                Types::StringPairVec    FormVars{ {"UUID", Id},
                                                  {"ERROR_TEXT", "This request does not contain a valid user ID. Please contact your system administrator."}};
                return SendHTMLFileBack(FormFile,FormVars);
            }

            if(UInfo.blackListed || UInfo.suspended) {
                Poco::File  FormFile{ Daemon()->AssetDir() + "/signup_verification_error.html"};
                Types::StringPairVec    FormVars{ {"UUID", Id},
                                                  {"ERROR_TEXT", "Please contact our system administrators. We have identified an error in your account that must be resolved first."}};
                return SendHTMLFileBack(FormFile,FormVars);
            }

            bool GoodPassword = AuthService()->SetSubPassword(Password1,UInfo);
            if(!GoodPassword) {
                Poco::File  FormFile{ Daemon()->AssetDir() + "/signup_verification_error.html"};
                Types::StringPairVec    FormVars{ {"UUID", Id},
                                                  {"ERROR_TEXT", "You cannot reuse one of your recent passwords."}};
                return SendHTMLFileBack(FormFile,FormVars);
            }

            UInfo.modified = std::time(nullptr);
            UInfo.changePassword = false;
            UInfo.lastEmailCheck = std::time(nullptr);
            UInfo.waitingForEmailCheck = false;

            StorageService()->SubDB().UpdateUserInfo(UInfo.email,Link.userId,UInfo);

            Poco::File  FormFile{ Daemon()->AssetDir() + "/signup_verification_success.html"};
            Types::StringPairVec    FormVars{ {"UUID", Id},
                                              {"USERNAME", UInfo.email} };
            StorageService()->ActionLinksDB().CompleteAction(Id);
            SendHTMLFileBack(FormFile,FormVars);
        } else {
            DoReturnA404();
        }
    }

    void RESTAPI_action_links::DoEmailVerification(SecurityObjects::ActionLink &Link) {
        auto Now = std::time(nullptr);

        if(Now > Link.expires) {
            StorageService()->ActionLinksDB().CancelAction(Link.id);
            return DoReturnA404();
        }

        SecurityObjects::UserInfo UInfo;
        bool Found = Link.userAction ? StorageService()->UserDB().GetUserById(Link.userId,UInfo) : StorageService()->SubDB().GetUserById(Link.userId,UInfo);
        if (!Found) {
            Types::StringPairVec FormVars{{"UUID",       Link.id},
                                          {"ERROR_TEXT", "This does not appear to be a valid email verification link.."}};
            Poco::File FormFile{Daemon()->AssetDir() + "/email_verification_error.html"};
            return SendHTMLFileBack(FormFile, FormVars);
        }

        Logger_.information(Poco::format("EMAIL-VERIFICATION(%s): For ID=%s", Request->clientAddress().toString(), UInfo.email));
        UInfo.waitingForEmailCheck = false;
        UInfo.validated = true;
        UInfo.lastEmailCheck = std::time(nullptr);
        UInfo.validationDate = std::time(nullptr);
        UInfo.modified  = std::time(nullptr);
        if(Link.userAction)
            StorageService()->UserDB().UpdateUserInfo(UInfo.email, Link.userId, UInfo);
        else
            StorageService()->SubDB().UpdateUserInfo(UInfo.email, Link.userId, UInfo);
        Types::StringPairVec FormVars{{"UUID",     Link.id},
                                      {"USERNAME", UInfo.email},
                                      {"ACTION_LINK",MicroService::instance().GetUIURI()}};
        Poco::File FormFile{Daemon()->AssetDir() + "/email_verification_success.html"};
        StorageService()->ActionLinksDB().CompleteAction(Link.id);
        SendHTMLFileBack(FormFile, FormVars);
    }

    void RESTAPI_action_links::DoReturnA404() {
        Types::StringPairVec FormVars;
        Poco::File FormFile{Daemon()->AssetDir() + "/404_error.html"};
        SendHTMLFileBack(FormFile, FormVars);
    }

}
