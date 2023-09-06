//
// Created by stephane bourque on 2021-06-22.
//

#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTMLForm.h"

#include "RESTAPI_action_links.h"
#include "StorageService.h"
#include "framework/OpenAPIRequests.h"
#include "framework/RESTAPI_PartHandler.h"

#include "Daemon.h"

namespace OpenWifi {

#if defined(TIP_CERT_SERVICE)
	bool ProcessExternalActionLinks(RESTAPIHandler &handler, const std::string &Id,
									const std::string &Action);
#endif

	void RESTAPI_action_links::DoGet() {

		auto Action = GetParameter("action", "");
		auto Id = GetParameter("id", "");

#if defined(TIP_CERT_SERVICE)
		if (!OpenWifi::ProcessExternalActionLinks(*this, Id, Action)) {
			return;
		}
#endif

		SecurityObjects::ActionLink Link;
		if (!StorageService()->ActionLinksDB().GetActionLink(Id, Link))
			return DoReturnA404();

		if (Action == "password_reset")
			return RequestResetPassword(Link);
		else if (Action == "sub_password_reset")
			return RequestSubResetPassword(Link);
		else if (Action == "email_verification")
			return DoEmailVerification(Link);
		else if (Action == "sub_email_verification")
			return DoSubEmailVerification(Link);
		else if (Action == "signup_verification")
			return DoNewSubVerification(Link);
		else
			return DoReturnA404();
	}

	void RESTAPI_action_links::DoPost() {
		auto Action = GetParameter("action", "");

		if (Action == "password_reset")
			return CompleteResetPassword();
		else if (Action == "sub_password_reset")
			return CompleteResetPassword();
		else if (Action == "signup_completion")
			return CompleteSubVerification();
		else if (Action == "email_invitation")
			return CompleteEmailInvitation();
		else
			return DoReturnA404();
	}

	void RESTAPI_action_links::AddGlobalVars(Types::StringPairVec &Vars) {
		Vars.push_back(std::make_pair("USER_HELPER_EMAIL", AuthService()->HelperEmail()));
		Vars.push_back(std::make_pair("SUB_HELPER_EMAIL", AuthService()->SubHelperEmail()));
		Vars.push_back(
			std::make_pair("GLOBAL_USER_HELPER_EMAIL", AuthService()->GlobalHelperEmail()));
		Vars.push_back(
			std::make_pair("GLOBAL_SUB_HELPER_EMAIL", AuthService()->GlobalSubHelperEmail()));
		Vars.push_back(std::make_pair("USER_HELPER_SITE", AuthService()->HelperSite()));
		Vars.push_back(std::make_pair("SUB_HELPER_SITE", AuthService()->SubHelperSite()));
		Vars.push_back(std::make_pair("USER_SYSTEM_LOGIN", AuthService()->SystemLoginSite()));
		Vars.push_back(std::make_pair("SUB_SYSTEM_LOGIN", AuthService()->SubSystemLoginSite()));
		Vars.push_back(std::make_pair("USER_SIGNATURE", AuthService()->UserSignature()));
		Vars.push_back(std::make_pair("SUB_SIGNATURE", AuthService()->SubSignature()));
	}

	void RESTAPI_action_links::RequestResetPassword(SecurityObjects::ActionLink &Link) {
		Logger_.information(fmt::format("REQUEST-PASSWORD-RESET({}): For ID={}",
										Request->clientAddress().toString(), Link.userId));
		Poco::File FormFile{Daemon()->AssetDir() + "/password_reset.html"};
		Types::StringPairVec FormVars{
			{"UUID", Link.id},
			{"PASSWORD_VALIDATION", AuthService()->PasswordValidationExpression()}};
		AddGlobalVars(FormVars);
		SendHTMLFileBack(FormFile, FormVars);
	}

	void RESTAPI_action_links::DoNewSubVerification(SecurityObjects::ActionLink &Link) {
		Logger_.information(fmt::format("REQUEST-SUB-SIGNUP({}): For ID={}",
										Request->clientAddress().toString(), Link.userId));
		Poco::File FormFile{Daemon()->AssetDir() + "/sub_signup_verification.html"};
		Types::StringPairVec FormVars{
			{"UUID", Link.id},
			{"PASSWORD_VALIDATION", AuthService()->PasswordValidationExpression()}};
		AddGlobalVars(FormVars);
		SendHTMLFileBack(FormFile, FormVars);
	}

	void RESTAPI_action_links::CompleteResetPassword() {
		RESTAPI_PartHandler PartHandler;
		Poco::Net::HTMLForm Form(*Request, Request->stream(), PartHandler);
		if (!Form.empty()) {

			auto Password1 = Form.get("password1", "bla");
			auto Password2 = Form.get("password2", "blu");
			auto Id = Form.get("id", "");
			auto now = OpenWifi::Now();

			SecurityObjects::ActionLink Link;
			if (!StorageService()->ActionLinksDB().GetActionLink(Id, Link))
				return DoReturnA404();

			if (now > Link.expires) {
				StorageService()->ActionLinksDB().CancelAction(Id);
				return DoReturnA404();
			}

			if (Password1 != Password2 || !AuthService()->ValidatePassword(Password2) ||
				!AuthService()->ValidatePassword(Password1)) {
				Poco::File FormFile{Daemon()->AssetDir() + "/password_reset_error.html"};
				Types::StringPairVec FormVars{
					{"UUID", Id},
					{"ERROR_TEXT",
					 "For some reason, the passwords entered do not match or they do not comply "
					 "with"
					 " accepted password creation restrictions. Please consult our on-line help"
					 " to look at the our password policy. If you would like to contact us, please "
					 "mention"
					 " id(" +
						 Id + ")"}};
				AddGlobalVars(FormVars);
				return SendHTMLFileBack(FormFile, FormVars);
			}

			SecurityObjects::UserInfo UInfo;

			bool Found = Link.userAction
							 ? StorageService()->UserDB().GetUserById(Link.userId, UInfo)
							 : StorageService()->SubDB().GetUserById(Link.userId, UInfo);
			if (!Found) {
				Poco::File FormFile{Daemon()->AssetDir() + "/password_reset_error.html"};
				Types::StringPairVec FormVars{
					{"UUID", Id},
					{"ERROR_TEXT", "This request does not contain a valid user ID. Please contact "
								   "your system administrator."}};
				AddGlobalVars(FormVars);
				return SendHTMLFileBack(FormFile, FormVars);
			}

			if (UInfo.blackListed || UInfo.suspended) {
				Poco::File FormFile{Daemon()->AssetDir() + "/password_reset_error.html"};
				Types::StringPairVec FormVars{
					{"UUID", Id},
					{"ERROR_TEXT", "Please contact our system administrators. We have identified "
								   "an error in your account that must be resolved first."}};
				AddGlobalVars(FormVars);
				return SendHTMLFileBack(FormFile, FormVars);
			}

			bool GoodPassword = Link.userAction ? AuthService()->SetPassword(Password1, UInfo)
												: AuthService()->SetSubPassword(Password1, UInfo);
			if (!GoodPassword) {
				Poco::File FormFile{Daemon()->AssetDir() + "/password_reset_error.html"};
				Types::StringPairVec FormVars{
					{"UUID", Id}, {"ERROR_TEXT", "You cannot reuse one of your recent passwords."}};
				AddGlobalVars(FormVars);
				return SendHTMLFileBack(FormFile, FormVars);
			}

			UInfo.modified = OpenWifi::Now();
			if (Link.userAction)
				StorageService()->UserDB().UpdateUserInfo(UInfo.email, Link.userId, UInfo);
			else
				StorageService()->SubDB().UpdateUserInfo(UInfo.email, Link.userId, UInfo);

			Poco::File FormFile{Daemon()->AssetDir() + "/password_reset_success.html"};
			Types::StringPairVec FormVars{{"UUID", Id},
										  {"USERNAME", UInfo.email},
										  {"ACTION_LINK", MicroService::instance().GetUIURI()}};
			AddGlobalVars(FormVars);
			StorageService()->ActionLinksDB().CompleteAction(Id);
			SendHTMLFileBack(FormFile, FormVars);
		} else {
			DoReturnA404();
		}
	}

	void RESTAPI_action_links::CompleteSubVerification() {
		RESTAPI_PartHandler PartHandler;
		Poco::Net::HTMLForm Form(*Request, Request->stream(), PartHandler);

		if (!Form.empty()) {
			auto Password1 = Form.get("password1", "bla");
			auto Password2 = Form.get("password2", "blu");
			auto Id = Form.get("id", "");
			auto now = OpenWifi::Now();

			SecurityObjects::ActionLink Link;
			if (!StorageService()->ActionLinksDB().GetActionLink(Id, Link)) {
				return DoReturnA404();
			}

			if (now > Link.expires) {
				StorageService()->ActionLinksDB().CancelAction(Id);
				return DoReturnA404();
			}

			if (Password1 != Password2 || !AuthService()->ValidateSubPassword(Password1)) {
				Poco::File FormFile{Daemon()->AssetDir() + "/sub_password_reset_error.html"};
				Types::StringPairVec FormVars{
					{"UUID", Id},
					{"ERROR_TEXT",
					 "For some reason, the passwords entered do not match or they do not comply "
					 "with"
					 " accepted password creation restrictions. Please consult our on-line help"
					 " to look at the our password policy. If you would like to contact us, please "
					 "mention"
					 " id(" +
						 Id + ")"}};
				AddGlobalVars(FormVars);
				return SendHTMLFileBack(FormFile, FormVars);
			}

			SecurityObjects::UserInfo UInfo;
			bool Found = StorageService()->SubDB().GetUserById(Link.userId, UInfo);
			if (!Found) {
				Poco::File FormFile{Daemon()->AssetDir() + "/sub_signup_verification_error.html"};
				Types::StringPairVec FormVars{
					{"UUID", Id},
					{"ERROR_TEXT", "This request does not contain a valid user ID. Please contact "
								   "your system administrator."}};
				AddGlobalVars(FormVars);
				return SendHTMLFileBack(FormFile, FormVars);
			}

			if (UInfo.blackListed || UInfo.suspended) {
				Poco::File FormFile{Daemon()->AssetDir() + "/sub_signup_verification_error.html"};
				Types::StringPairVec FormVars{
					{"UUID", Id},
					{"ERROR_TEXT", "Please contact our system administrators. We have identified "
								   "an error in your account that must be resolved first."}};
				AddGlobalVars(FormVars);
				return SendHTMLFileBack(FormFile, FormVars);
			}

			bool GoodPassword = AuthService()->SetSubPassword(Password1, UInfo);
			if (!GoodPassword) {
				Poco::File FormFile{Daemon()->AssetDir() + "/sub_signup_verification_error.html"};
				Types::StringPairVec FormVars{
					{"UUID", Id}, {"ERROR_TEXT", "You cannot reuse one of your recent passwords."}};
				AddGlobalVars(FormVars);
				return SendHTMLFileBack(FormFile, FormVars);
			}

			UInfo.modified = OpenWifi::Now();
			UInfo.changePassword = false;
			UInfo.lastEmailCheck = OpenWifi::Now();
			UInfo.waitingForEmailCheck = false;
			UInfo.validated = OpenWifi::Now();

			StorageService()->SubDB().UpdateUserInfo(UInfo.email, Link.userId, UInfo);

			Poco::File FormFile{Daemon()->AssetDir() + "/sub_signup_verification_success.html"};
			Types::StringPairVec FormVars{{"UUID", Id}, {"USERNAME", UInfo.email}};
			StorageService()->ActionLinksDB().CompleteAction(Id);

			//  Send the update to the provisioning service
			Poco::JSON::Object Body;
			auto RawSignup = Poco::StringTokenizer(UInfo.signingUp, ":");
			Body.set("signupUUID", RawSignup.count() == 1 ? UInfo.signingUp : RawSignup[1]);
			OpenAPIRequestPut ProvRequest(
				uSERVICE_PROVISIONING, "/api/v1/signup",
				{{"signupUUID", RawSignup.count() == 1 ? UInfo.signingUp : RawSignup[1]},
				 {"operation", "emailVerified"}},
				Body, 30000);
			Logger().information(fmt::format(
				"({}): Completed subscriber e-mail verification and password.", UInfo.email));
			Poco::JSON::Object::Ptr Response;
			auto Status = ProvRequest.Do(Response);
			std::stringstream ooo;
			if (Response != nullptr)
				Response->stringify(ooo);
			Logger().information(fmt::format(
				"({}): Completed subscriber e-mail verification. Provisioning notified, Error={}.",
				UInfo.email, Status));
			AddGlobalVars(FormVars);
			SendHTMLFileBack(FormFile, FormVars);
			Logger().information(fmt::format(
				"({}): Completed subscriber e-mail verification. FORM notified.", UInfo.email));
		} else {
			DoReturnA404();
		}
	}

	void RESTAPI_action_links::DoEmailVerification(SecurityObjects::ActionLink &Link) {
		auto now = OpenWifi::Now();

		if (now > Link.expires) {
			StorageService()->ActionLinksDB().CancelAction(Link.id);
			return DoReturnA404();
		}

		SecurityObjects::UserInfo UInfo;
		bool Found = Link.userAction ? StorageService()->UserDB().GetUserById(Link.userId, UInfo)
									 : StorageService()->SubDB().GetUserById(Link.userId, UInfo);
		if (!Found) {
			Types::StringPairVec FormVars{
				{"UUID", Link.id},
				{"ERROR_TEXT", "This does not appear to be a valid email verification link.."}};
			Poco::File FormFile{Daemon()->AssetDir() + "/email_verification_error.html"};
			AddGlobalVars(FormVars);
			return SendHTMLFileBack(FormFile, FormVars);
		}

		Logger_.information(fmt::format("EMAIL-VERIFICATION(%s): For ID={}",
										Request->clientAddress().toString(), UInfo.email));
		UInfo.waitingForEmailCheck = false;
		UInfo.validated = true;
		UInfo.lastEmailCheck = OpenWifi::Now();
		UInfo.validationDate = OpenWifi::Now();
		UInfo.modified = OpenWifi::Now();
		if (Link.userAction)
			StorageService()->UserDB().UpdateUserInfo(UInfo.email, Link.userId, UInfo);
		else
			StorageService()->SubDB().UpdateUserInfo(UInfo.email, Link.userId, UInfo);
		Types::StringPairVec FormVars{{"UUID", Link.id},
									  {"USERNAME", UInfo.email},
									  {"ACTION_LINK", MicroService::instance().GetUIURI()}};
		Poco::File FormFile{Daemon()->AssetDir() + "/email_verification_success.html"};
		AddGlobalVars(FormVars);
		StorageService()->ActionLinksDB().CompleteAction(Link.id);
		SendHTMLFileBack(FormFile, FormVars);
	}

	void RESTAPI_action_links::DoReturnA404() {
		Types::StringPairVec FormVars;
		Poco::File FormFile{Daemon()->AssetDir() + "/404_error.html"};
		AddGlobalVars(FormVars);
		SendHTMLFileBack(FormFile, FormVars);
	}

	void RESTAPI_action_links::CompleteEmailInvitation() {
		/// TODO:
	}

	void RESTAPI_action_links::RequestSubResetPassword(
		[[maybe_unused]] SecurityObjects::ActionLink &Link) {}

	void RESTAPI_action_links::DoSubEmailVerification(
		[[maybe_unused]] SecurityObjects::ActionLink &Link) {}

} // namespace OpenWifi
