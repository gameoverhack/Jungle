#include "GraphicLoader.h"

GraphicLoader::GraphicLoader(string assetPath) {

    _dataPath = ofToDataPath(assetPath);

    LOG_NOTICE("Start loading graphic assets from path: " + _dataPath);
    _lister.allowExt("png");
    _lister.listDir(_dataPath);

    for (int i = 0; i < _lister.size(); i++) {

        string name = _lister.getName(i);
        string path = assetPath+"/"+name;

        LOG_VERBOSE("Attempt to load: " + path + "...");

        bool ok = false;

        // ok so this method is crap but hey it's just 9 graphics files ;-)
        if (name.find("meter_level")     != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_METER_LEVEL);
        if (name.find("meter_off")       != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_METER_OFF);
        if (name.find("meter_on")        != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_METER_ON);
        if (name.find("punch_no")        != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_PUNCH_OFF);
        if (name.find("punch_bar")    != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_PUNCH_BAR);
        if (name.find("punch_yes")       != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_PUNCH_ON);
        if (name.find("scream_no")       != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_SCREAM_OFF);
        if (name.find("scream_bar")   != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_SCREAM_BAR);
        if (name.find("scream_yes")      != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_SCREAM_ON);

        LOG_VERBOSE((string)(ok ? "ok" : "fail"));
        assert(ok);
    }

}

GraphicLoader::~GraphicLoader() {
    //dtor
}
