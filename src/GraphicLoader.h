#ifndef _H_GRAPHICLOADER
#define _H_GRAPHICLOADER

#include "goDirList.h"
#include "AppModel.h"
#include "Logger.h"

class GraphicLoader {

public:

    GraphicLoader(string assetPath);
    ~GraphicLoader();

private:

    string          _dataPath;
    goDirList		_lister;

};

#endif // _H_GRAPHICLOADER
