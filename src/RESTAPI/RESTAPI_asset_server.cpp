//
// Created by stephane bourque on 2021-07-10.
//

#include "RESTAPI_asset_server.h"
#include "Daemon.h"
#include "Poco/File.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	void RESTAPI_asset_server::DoGet() {
		Poco::File AssetFile;

		if (Request->getURI().find("/favicon.ico") != std::string::npos) {
			AssetFile = Daemon()->AssetDir() + "/favicon.ico";
		} else {
			std::string AssetName = GetBinding(RESTAPI::Protocol::ID, "");
			AssetFile = Daemon()->AssetDir() + "/" + AssetName;
		}
		if (!AssetFile.isFile()) {
			return NotFound();
		}
		SendFile(AssetFile);
	}
} // namespace OpenWifi