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
            SUB_VERIFICATION_CODE,
            CERTIFICATE_TRANSFER_NOTIFICATION,
            CERTIFICATE_TRANSFER_AUTHORIZATION,
            CERTIFICATE_DISPUTE_SUCCESS,
            CERTIFICATE_DISPUTE_REJECTED,
            CERTIFICATE_TRANSFER_CANCELED,
            CERTIFICATE_TRANSFER_ACCEPTED,
            CERTIFICATE_TRANSFER_REJECTED
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
                case CERTIFICATE_TRANSFER_NOTIFICATION: return AddOperator(EmailTemplateNames[CERTIFICATE_TRANSFER_NOTIFICATION],OperatorName);
                case CERTIFICATE_TRANSFER_AUTHORIZATION: return AddOperator(EmailTemplateNames[CERTIFICATE_TRANSFER_AUTHORIZATION],OperatorName);
                case CERTIFICATE_DISPUTE_SUCCESS: return AddOperator(EmailTemplateNames[CERTIFICATE_DISPUTE_SUCCESS],OperatorName);
                case CERTIFICATE_DISPUTE_REJECTED: return AddOperator(EmailTemplateNames[CERTIFICATE_DISPUTE_REJECTED],OperatorName);
                case CERTIFICATE_TRANSFER_CANCELED: return AddOperator(EmailTemplateNames[CERTIFICATE_TRANSFER_CANCELED],OperatorName);
                case CERTIFICATE_TRANSFER_ACCEPTED: return AddOperator(EmailTemplateNames[CERTIFICATE_TRANSFER_ACCEPTED],OperatorName);
                case CERTIFICATE_TRANSFER_REJECTED: return AddOperator(EmailTemplateNames[CERTIFICATE_TRANSFER_REJECTED],OperatorName);
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
                "sub_verification_code",
                "certificate_transfer_notification",
                "certificate_transfer_authorization",
                "certificate_dispute_success",
                "certificate_dispute_rejected",
                "certificate_transfer_canceled",
                "certificate_transfer_accepted",
                "certificate_transfer_rejected"
        };
    };

    inline MessagingTemplates & MessagingTemplates() { return MessagingTemplates::instance(); }

} // OpenWifi

