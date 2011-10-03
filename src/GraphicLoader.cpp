#include "GraphicLoader.h"

GraphicLoader::GraphicLoader(string assetPath) {

    _dataPath = ofToDataPath(assetPath);

    LOG_NOTICE("Start loading graphic assets from path: " + _dataPath);
    _lister.allowExt("png");
    _lister.listDir(_dataPath);

    for (int i = 0; i < _lister.size(); i++) {

        string name = _lister.getName(i);
        string path = assetPath+"/"+name;
        string msg =    "Attempt to load: " + path + "...";

        LOG_VERBOSE(msg);
        _appModel->setProperty("loadingMessage", msg);

        bool ok = false;

        // ok so this method is crap but hey it's just ~9 graphics files ;-)
        if (name.find("arrows_off")     != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_ARROWS_ON);
        if (name.find("arrows_on")      != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_ARROWS_OFF);
        if (name.find("meter_on")       != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_METER_ON);
        if (name.find("meter_off")      != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_METER_OFF);
        if (name.find("button_on")      != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_BUTTON_ON);
        if (name.find("button_off")     != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_BUTTON_OFF);
        if (name.find("button_no")      != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_BUTTON_DENY);
        if (name.find("top_on")         != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_TOP_ON);
        if (name.find("top_off")        != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_TOP_OFF);
        if (name.find("top_no")         != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_TOP_DENY);
        if (name.find("stations_on")    != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_STATIONS_ON);
        if (name.find("stations_off")   != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_STATIONS_OFF);
        if (name.find("bird")           != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_BIRD);
        if (name.find("turtle")         != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_TURTLE);
        if (name.find("bar")            != string::npos) ok = _appModel->loadGraphicAsset(path, kGFX_TURTLE_BAR);

        msg = (string)(ok ? "ok" : "fail");
        LOG_VERBOSE(msg);
        _appModel->setProperty("loadingMessage", msg);
        assert(ok);
    }

}

GraphicLoader::~GraphicLoader() {
    //dtor
}
