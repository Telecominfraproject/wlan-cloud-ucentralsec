//
// Created by stephane bourque on 2022-07-25.
//

#pragma once

#include <string>
#include <vector>

namespace OpenWifi {

    class MessagingTemplates {
    public:
        static MessagingTemplates & instance() {
            static auto instance = new MessagingTemplates;
            return *instance;
        }

        enum EMAIL_REASON {
            FORGOT_PASSWORD = 0,
            EMAIL_VERIFICATION,
            SUB_SIGNUP_VERIFICATION,
            EMAIL_INVITATION,
            VERIFICATION_CODE,
            SUB_FORGOT_PASSWORD,
            SUB_EMAIL_VERIFICATION,
            SUB_VERIFICATION_CODE
        };

        static std::string AddOperator(const std::string & filename, const std::string &OperatorName) {
            if(OperatorName.empty())
                return "/" + filename;
            return "/" + OperatorName + "/" + filename;
        }

        static std::string TemplateName( EMAIL_REASON r , const std::string &OperatorName="") {
            switch (r) {
                case FORGOT_PASSWORD: return AddOperator(EmailTemplateNames[FORGOT_PASSWORD],OperatorName);
                case EMAIL_VERIFICATION: return AddOperator(EmailTemplateNames[EMAIL_VERIFICATION],OperatorName);
                case SUB_SIGNUP_VERIFICATION: return AddOperator(EmailTemplateNames[SUB_SIGNUP_VERIFICATION],OperatorName);
                case EMAIL_INVITATION: return AddOperator(EmailTemplateNames[EMAIL_INVITATION],OperatorName);
                case VERIFICATION_CODE: return AddOperator(EmailTemplateNames[VERIFICATION_CODE],OperatorName);
                case SUB_FORGOT_PASSWORD: return AddOperator(EmailTemplateNames[SUB_FORGOT_PASSWORD],OperatorName);
                case SUB_EMAIL_VERIFICATION: return AddOperator(EmailTemplateNames[SUB_EMAIL_VERIFICATION],OperatorName);
                case SUB_VERIFICATION_CODE: return AddOperator(EmailTemplateNames[SUB_VERIFICATION_CODE],OperatorName);
                default:
                    return "";
            }
        }

        static std::string Logo(const std::string &OperatorName = "" ) {
            return AddOperator("logo.png", OperatorName);
        }

        static std::string SubLogo(const std::string &OperatorName = "" ) {
            return AddOperator("sub_logo.png", OperatorName);
        }

    private:
        inline const static std::vector<std::string>  EmailTemplateNames = {
                "password_reset",
                "email_verification",
                "sub_signup_verification",
                "email_invitation",
                "verification_code",
                "sub_password_reset",
                "sub_email_verification",
                "sub_verification_code"
        };
    };

    inline MessagingTemplates & MessagingTemplates() { return MessagingTemplates::instance(); }

} // OpenWifi

