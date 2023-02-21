//
// Created by stephane bourque on 2023-01-25.
//

#pragma once

#include <framework/SubSystemServer.h>

namespace OpenWifi {

	class SecretStore : public SubSystemServer {
	  public:
		using SecretStoreType = std::map<std::string, std::string>;
		static SecretStore *instance() {
			static auto *instance_ = new SecretStore;
			return instance_;
		}

		int Start() final;
		void Stop() final;
		void ReadStore();
		void SaveStore();
		bool Get(const std::string &key, std::string &value, const std::string &default_value);
		void Set(const std::string &key, const std::string &value);
		void Remove(const std::string &key);
		inline SecretStoreType Store() {
			std::lock_guard G(Mutex_);
			return Store_;
		}

	  private:
		SecretStoreType Store_;
		SecretStore() noexcept : SubSystemServer("SecretStore", "SECRET-SVR", "secret.store") {}
	};
	inline SecretStore *SecretStore() { return SecretStore::instance(); }

} // namespace OpenWifi
