/*
 *  SceneXMLBuilder.cpp
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "SceneXMLBuilder.h"

void listStuff(map<string, map<string, string> > m){
	map<string, map<string, string> >::iterator iter;
	iter = m.begin();
	while (iter != m.end()) {
		printf("%s =| \n", (iter->first).c_str());
		map<string, string>::iterator iter2;
		iter2 = iter->second.begin();
		while(iter2 != iter->second.end()){
			printf("\t%s => %s\n", (iter2->first).c_str(), (iter2->second).c_str());
			iter2++;
		}
		iter++;
	}
}

SceneXMLBuilder::SceneXMLBuilder(string dataPath, string xmlFile) : IXMLBuilder(xmlFile) {

	LOG_NOTICE("Creating scene xml builder with path:" + dataPath + " and config:" + xmlFile);
	_dataPath = dataPath;

	// some pre-flight setup
	setupFileListers();	// set up lister
	santiseFiles();	// make sure files are ok

	LOG_NOTICE("Checking file validity, frame lengths etc");
	// check file validity (frame lengths etc)
	checkMovieAssets();

	buildAppModel();

	buildXML();		// build xml from app model
	if(!save()){	// save xml
		LOG_ERROR("Could not save XML");
		abort();
	};
}

// Set up file lister
// resets state so it can be called whenever you want to start fresh
void SceneXMLBuilder::setupFileListers(){
	_moviesFileLister.reset();
	_moviesFileLister.addDir(_dataPath);
	_moviesFileLister.allowExt("mov");
	_moviesFileLister.allowExt("MOV");
	_moviesFileLister.listDir(true);


	_assetsFileLister.reset();
	_assetsFileLister.addDir(_dataPath);
	_assetsFileLister.allowExt("bin");
	_assetsFileLister.allowExt("BIN");
	_assetsFileLister.listDir(true);

	// quick error check
	if(_moviesFileLister.size() == 0){
		LOG_ERROR("Could not build scene config, goDirList reported 0 movie files found.");
		abort();
	}

	if(_assetsFileLister.size() == 0){
		LOG_ERROR("Could not build scene config, goDirList reported 0 asset files found.");
		//throw JungleException("0 asset files found");
	}
}

// Makes sure our files are in good order
// This means all lowercase.
// (Anything else required?)
void SceneXMLBuilder::santiseFiles(){
	// Lowercase all filenames
	string filename;
	string filenameLower;
	string path, pathLower;
	for(int fileNum = 0; fileNum < _moviesFileLister.size(); fileNum++){
		// get the file name
		filename = _moviesFileLister.getName(fileNum);
		filenameLower = filename; // have to copy, setting the output iterator in transform didnt work...
		std::transform(filenameLower.begin(), filenameLower.end(), filenameLower.begin(), ::tolower);
		path = _moviesFileLister.getPath(fileNum);
		pathLower = path;
		pathLower.replace(pathLower.find_last_of(filename)-filename.length()+1, filename.length(), filenameLower);
		rename(path.c_str(), pathLower.c_str());
	}
	for(int fileNum = 0; fileNum < _assetsFileLister.size(); fileNum++){
		// get the file name
		filename = _assetsFileLister.getName(fileNum);
		filenameLower = filename; // have to copy, setting the output iterator in transform didnt work...
		std::transform(filenameLower.begin(), filenameLower.end(), filenameLower.begin(), ::tolower);
		path = _assetsFileLister.getPath(fileNum);
		pathLower = path;
		pathLower.replace(pathLower.find_last_of(filename)-filename.length()+1, filename.length(), filenameLower);
		rename(path.c_str(), pathLower.c_str());
	}

	// have to relist dir because filenames have changed (possibly)
	setupFileListers();
}


// checks that movie files have transforms files and an interactivity file
void SceneXMLBuilder::checkMovieAssets() {

	int assetFileId;
	string movieFilename, assetFilename, assetPath;
	vector<string> brokenFiles; // list of things that need reanalisis
	vector<string> assetFilenameParts;
	_movieFrameLengths.clear();

	ofxThreadedVideo movie;

	SequenceDescriptor *seqDescriptor = new SequenceDescriptor();
	vector<CamTransform> transform;

	for(int movieFileId = 0; movieFileId < _moviesFileLister.size(); movieFileId++){

		// get name without extension
		movieFilename = _moviesFileLister.getNameWithoutExtension(movieFileId);

		LOG_NOTICE("Loading movie: " + movieFilename);
		movie.setUseTexture(false);
		movie.loadMovie(_moviesFileLister.getPath(movieFileId));
        _movieFrameLengths.insert(pair<string, int>(_moviesFileLister.getName(movieFileId), movie.getTotalNumFrames()));

		assetFilenameParts.clear();
		assetFilenameParts.push_back("_interactivity.bin");
		assetFilenameParts.push_back("_transform_vic1.bin");
		assetFilenameParts.push_back("_transform_atk1.bin");
		if(movieFilename.find("t_") == 0){
			 // FIXME: Check if start of name is t_ and add then?
			assetFilenameParts.push_back("_transform_atk2.bin");
		}

		for(vector<string>::iterator iter = assetFilenameParts.begin();	iter != assetFilenameParts.end(); iter++){

			// check for interactifity file
			assetFilename = movieFilename+*iter;
			LOG_NOTICE("Checking for " + assetFilename);
			assetFileId = _assetsFileLister.findFileByName(assetFilename);
			if(assetFileId == -1){
				LOG_NOTICE("Did not find");
				brokenFiles.push_back(assetFilename); // does not exist
			}
			else{
				assetPath = _assetsFileLister.getPath(assetFileId);
				// try to load files
				if(iter->find("_interactivity") != iter->npos){
					// found interactivity file
					loadClass(assetPath, seqDescriptor);
					// LOG_NOTICE("Loaded interactiviity file");
					// We don't actually check this as long as the files there we assume its ok.
					// if the interactivity doesnt cover the whole length it defaults to no interactivity anyway.
				} else {
					// transform file
					transform.clear();
					loadVector(assetPath, &transform);
					if(transform.size() != movie.getTotalNumFrames()){
						LOG_WARNING("Frame count mismatch for "+ assetFilename + " and " + movieFilename);
						brokenFiles.push_back(assetFilename); // exists but broken
					}
				}
			}
		}
	}

	// clean up
	delete seqDescriptor;

	if(brokenFiles.size() != 0){
		throw AnalysisRequiredException("Missing files", brokenFiles);
	}
}

void SceneXMLBuilder::buildAppModel(){
	// used for object construction
	Scene *scene = NULL;
	Sequence *sequence = NULL;
	vector<CamTransform> *transform = NULL;
	ofxThreadedVideo *movie;
    int totalFrames = 0;
	string movieFilename;
	for(int movieFileId = 0; movieFileId < _moviesFileLister.size(); movieFileId++){
		movieFilename = _moviesFileLister.getNameWithoutExtension(movieFileId);
		vector<string> movieFilenameParts;
		boost::split(movieFilenameParts, movieFilename, boost::is_any_of("_"));
		// first part of name is scene, get/create
		if(_builderModel.find(movieFilenameParts[0]) == _builderModel.end()){
			scene = new Scene();
			scene->setName(movieFilenameParts[0]);
			_builderModel.insert(make_pair(movieFilenameParts[0], scene));
			totalFrames = 0;
			scene->setTotalFrames(0);
		}
		else{
			scene = _builderModel.find(movieFilenameParts[0])->second;
			totalFrames = scene->getTotalFrames();
		}
		// next part is sequence name
		sequence = new Sequence();

		// t_seq01a
		sequence->setName(movieFilenameParts[1]); //t_[seq01a]

		int sequenceNumber = findSequenceNumberFromString(sequence->getName());

		sequence->setNumber(sequenceNumber); //t_seq[01]a

		//sequence->setNextSequenceName("_not_forging_");
		//sequence->setFaceResult("_not_forging_");
		//sequence->setAttackerResult("_not_forging_");
		//sequence->setVictimResult("_not_forging_");

        int frames = _movieFrameLengths[_moviesFileLister.getName(movieFileId)];

		if(regex_search(movieFilename, regex("_loop"))){
			// is a loop movie
			sequence->setType("loop");
			// fix up name (we chopped off the _loop part
			sequence->setName(movieFilenameParts[1]+"_loop");
		}
		else{
			if(regex_search(movieFilenameParts[1], regex("a$"))){
				sequence->setType("a"); // t_seq01[a]
				if (sequenceNumber > 0) totalFrames += frames; // don't include loops, bs or the first ("waiting") sequence in the count
			}
			else{
			   sequence->setType("b");
			}
		}

		sequence->setMovieFullFilePath(_moviesFileLister.getPath(movieFileId));
		sequence->setNumFrames(frames);
		sequence->_interactivityFilename = movieFilename+"_interactivity.bin";
		sequence->_transformsFilenames.push_back(movieFilename+"_transform_vic1.bin");
		sequence->_transformsFilenames.push_back(movieFilename+"_transform_atk1.bin");
		if(scene->getName() == "t"){// FIXME: Check if scene is t and add this?
			sequence->_transformsFilenames.push_back(movieFilename+"_transform_atk2.bin");
		}

        scene->setTotalFrames(totalFrames);
		scene->setSequence(sequence->getName(), sequence);
	}

	// setup iterators etc
	map<string, Scene*> scenes = _builderModel;
	map<string, Scene*>::iterator scenei = scenes.begin();
	map<string, Sequence*> sequences;
	map<string, Sequence*>::iterator seqi;
	map<string, Sequence*>::reverse_iterator rseqi;

/*#pragma mark FORGE SETTINGS
	// _ FORGING flag _
	bool forgefinals = true; // final sequences
	bool forgenexts = false; // next sequences
	bool forgemissing = false; // fix missing sequences if any are referenced

	// setup final sequences
	if(forgefinals){
		while(scenei != scenes.end()){
			scene = scenei->second;
			sequences = scene->getSequences();
			bool a,b;
			a = b = false;
			rseqi = sequences.rbegin();
			while(true){
				sequence = rseqi->second;
				if(sequence->getType() == "a"){
					sequence->setNextSequenceName(kLAST_SEQUENCE_TOKEN);
					sequence->setFaceResult(kLAST_SEQUENCE_TOKEN);
					sequence->setAttackerResult(kLAST_SEQUENCE_TOKEN);
					sequence->setVictimResult(kLAST_SEQUENCE_TOKEN);
					a = true;
				}
				if(sequence->getType() == "b"){

					sequence->setNextSequenceName(kLAST_SEQUENCE_TOKEN);
					sequence->setFaceResult(kLAST_SEQUENCE_TOKEN);
					sequence->setAttackerResult(kLAST_SEQUENCE_TOKEN);
					sequence->setVictimResult(kLAST_SEQUENCE_TOKEN);
					b = true;
				}
				if(a && b){
					// set a and b finals
					break;
				}
				rseqi++;
				if(rseqi == sequences.rend()){
					sequence = sequences.rbegin()->second;
					// save absolute last as final sequence since we found no a
					sequence->setNextSequenceName(kLAST_SEQUENCE_TOKEN);
					sequence->setFaceResult(kLAST_SEQUENCE_TOKEN);
					sequence->setAttackerResult(kLAST_SEQUENCE_TOKEN);
					sequence->setVictimResult(kLAST_SEQUENCE_TOKEN);
					break;
				}
			}
			scenei++;
		}
	}

	// setup guessed next sequences
	if(forgenexts){
		// TODO: setup guessed next sequences
	}

	// fix missing scenes etc
	if(forgemissing){
		// TODO: fix missing referenced scenes if they are missing
	}

	// Check that the sequences for all scenes are sequential
	// iterate all scenes
	while(scenei != scenes.end()){
		scene = scenei->second;
		sequences = scene->getSequences();
		seqi = sequences.begin();
		int i = -1; // comparitor
		// iterate the sequences in this scene
		while(seqi != sequences.end()){
			sequence =seqi->second;
			// FIXME: Sequential sequences check
			// seq00a and seq00a_loop both have a sequence number of 0.
			// we cant do this check because of that.
			// you could either give all sequences a unique ID when you create
			// them irrespecitve of the sequence number in their name
			// (pointless since it would defeat the purpose of this test)
			// or ignore loops or something (sequence->getType() == "loop")
			if((i+1) != sequence->getNumber()){
				LOG_ERROR("Sequence number was not sequential for "+ scene->getName() + "->" + sequence->getName());
//				abort(); // commended out because we can't check properly as described above.
			}
			i++;
			seqi++;
		}
		scenei++;
	}
	*/
}

void SceneXMLBuilder::buildXML(){
	map<string, string> fileInfo;
	map<string, Scene*> scenes = _builderModel;
	map<string, Sequence*> sequences;
	map<string, Sequence*>::iterator seqi;
	map<string, Scene*>::iterator scenei = scenes.begin();
	Scene *scene;
	Sequence * sequence;
	int fileId;
	int which; // used for ofxXmlSettings "which" params

	// set up xml basics
	_xml.addTag("config:scenes");

	// set document root
	// note both these are popped at end of method
	_xml.pushTag("config");
	_xml.pushTag("scenes");

	// iterate all scenes
	while(scenei != scenes.end()){
		scene = scenei->second;
		sequences = scene->getSequences();

		which = _xml.addTag("scene");
		_xml.addAttribute("scene", "name", scene->getName(), which);
		_xml.addAttribute("scene", "totalFrames", scene->getTotalFrames(), which);
		seqi = sequences.begin();
		_xml.pushTag("scene", which);
		// iterate the sequences in this scene
		while(seqi != sequences.end()){
			sequence =seqi->second;

			which = _xml.addTag("sequence");
			_xml.addAttribute("sequence", "name", sequence->getName(), which);
			_xml.addAttribute("sequence", "sequenceType", sequence->getType(), which);
			//_xml.addAttribute("sequence", "faceResult", sequence->getFaceResult(), which);
			//_xml.addAttribute("sequence", "attackerResult", sequence->getAttackerResult(), which);
			//_xml.addAttribute("sequence", "victimResult", sequence->getVictimResult(), which);
			//_xml.addAttribute("sequence", "nextSequence", sequence->getNextSequenceName(), which);
			_xml.addAttribute("sequence", "faked", (sequence->getIsSequenceFaked() ? "true" : "false"), which);

			string movieFilename = sequence->getMovieFullFilePath();
			movieFilename = movieFilename.substr(movieFilename.rfind(scene->getName()+"_"));
			_xml.addAttribute("sequence", "filename", movieFilename, which);
			_xml.addAttribute("sequence", "frames", sequence->getNumFrames(), which);
			fileId = _moviesFileLister.findFileByName(movieFilename);
			_xml.addAttribute("sequence", "dateCreated", _moviesFileLister.getCreated(fileId), which);
			_xml.addAttribute("sequence", "dateModified", _moviesFileLister.getModified(fileId), which);
			_xml.addAttribute("sequence", "size", _moviesFileLister.getSize(fileId), which);

			_xml.pushTag("sequence", which);

			which = _xml.addTag("interactivity");
			_xml.addAttribute("interactivity", "filename", sequence->_interactivityFilename, which);
			fileId = _assetsFileLister.findFileByName(sequence->_interactivityFilename);
			_xml.addAttribute("interactivity", "dateCreated", _assetsFileLister.getCreated(fileId), which);
			_xml.addAttribute("interactivity", "dateModified", _assetsFileLister.getModified(fileId), which);
			_xml.addAttribute("interactivity", "size", _assetsFileLister.getSize(fileId), which);

			vector<string>::iterator iter = sequence->_transformsFilenames.begin();
			while(iter != sequence->_transformsFilenames.end()){
				which = _xml.addTag("transform");
				_xml.addAttribute("transform", "filename", *iter, which);
				fileId = _assetsFileLister.findFileByName(*iter);
				_xml.addAttribute("transform", "dateCreated", _assetsFileLister.getCreated(fileId), which);
				_xml.addAttribute("transform", "dateModified", _assetsFileLister.getModified(fileId), which);
				_xml.addAttribute("transform", "size", _assetsFileLister.getSize(fileId), which);
				iter++;
			}

			_xml.popTag();
			seqi++;
		}
		_xml.popTag();
		scenei++;
	}
}

/*




		Saved for later inclusion.





 */
// Scans files in directories, working out information relavent to them.
//void SceneXMLBuilder::findAndFixInvalidSequences() {
//	set<string> invalids;
//
//	map<string, map<string, string> >::iterator iter = _info.begin();
//	while(iter != _info.end()){
//		map<string, string> & kvmap = (iter->second); // syntax convenience
//
//		string scene = kvmap["scene"];
//		string checkKey;
//		LOG_VERBOSE("FindFixInvalid: " + iter->first);
//		if(kvmap["type"] == "loop"){
//			LOG_VERBOSE("\tLoop type");
//			//checkKey = scene + ":" + kvmap["attackerResult"];
//			LOG_VERBOSE("\tfind "+checkKey);
//			// do the find
//			if(_info.find(checkKey) == _info.end()){
//				LOG_VERBOSE("\t\tDid not find! Adding to invalids");
//				// is invalid, so store in invalid vector
//				invalids.insert(checkKey);
//			}
//			//checkKey = scene + ":" +  kvmap["victimResult"];
//			LOG_VERBOSE("\tfind "+checkKey);
//			if(_info.find(checkKey) == _info.end()){
//				LOG_VERBOSE("\t\tDid not find! Adding to invalids");
//				// is invalid, so store in invalid vector
//				invalids.insert(checkKey);
//			}
//		}
//
//		// Note, we ignore things when they are set to kLAST_SEQUENCE_TOKEN
//		// this is because the last a sequence doesn't have a loop after it
//		// We can be certain that the data in kvmap isn't corrupt because we've
//		// just created it ourself.
//		if(kvmap["type"] == "a"){
//			LOG_VERBOSE("\ta seq type");
//			// check victimResult and next sequence
//			if(kvmap["victimResult"] != kLAST_SEQUENCE_TOKEN){
//				checkKey = scene + ":" +  kvmap["victimResult"];
//				LOG_VERBOSE("\tfind "+checkKey);
//				// do the find
//				if(_info.find(checkKey) == _info.end()){
//					LOG_VERBOSE("\t\tDid not find! Adding to invalids");
//					// is invalid, so store in invalid vector
//					invalids.insert(checkKey);
//				}
//			}
//			if(kvmap["nextSequence"] != kLAST_SEQUENCE_TOKEN){
//				checkKey = scene + ":" +  kvmap["nextSequence"];
//				LOG_VERBOSE("\tfind "+checkKey);
//				if(_info.find(checkKey) == _info.end()){
//					LOG_VERBOSE("\t\tDid not find! Adding to invalids");
//					// is invalid, so store in invalid vector
//					invalids.insert(checkKey);
//				}
//			}
//		}
//		if(kvmap["type"] == "b"){
//			LOG_VERBOSE("\b seq type");
//			if(kvmap["nextSequence"] != kLAST_SEQUENCE_TOKEN){
//				LOG_WARNING(kvmap["name"] + " nextSequence was not __FINAL_SEQUENCE_, was " + kvmap["nextSequence"] + ", Fixing");
//				kvmap["nextSequence"] = kLAST_SEQUENCE_TOKEN;
//			}
//		}
//		iter++;
//	}
//
//	// We now have a list of invalid sequence references,
//	// so we'll try to construct some of these by cloning similar things.
//	// we also have to clone the transforms for whatever we copy
//	for(set<string>::iterator iter = invalids.begin(); iter != invalids.end(); iter++){
//		string fullInvalidKey = *iter;
//		vector<string> invalidKeyParts;
//		boost::split(invalidKeyParts, fullInvalidKey, boost::is_any_of(":"));
//
//		// set up the regex
//		string regexPattern = invalidKeyParts[0] + ":seq\\d+";
//
//		if(regex_search(fullInvalidKey, regex("b$"))){
//			// b key
//			regexPattern = regexPattern + "b$";
//		}
//		if(regex_search(fullInvalidKey, regex("a$"))){
//			// a key
//			regexPattern = regexPattern + "a$";
//		}
//		if(regex_search(fullInvalidKey, regex("_loop$"))){
//			// loop key
//			regexPattern = regexPattern + ".+?_loop$$";
//		}
//
//		// stores our clone targets,
//		// invalids => valids
//		map<string, string> matches;
//
//		string match = "";
//
//		// iterate over all the keys we have.
//		for(map<string, map<string, string> >::iterator infoiter = _info.begin(); infoiter != _info.end(); infoiter++){
//			// check this against our regex pattern
//			if(regex_search(infoiter->first, regex(regexPattern))){
//				// save our key (dont clone here because we shouldn't edit map during iteration.
//				match = infoiter->first;
//				break;
//			}
//		}
//
//		// we might not find a possible fix
//		if(match == ""){
//			LOG_ERROR("Sequence " + fullInvalidKey + " does not exist, could not find target to clone for fake");
//			throw JungleException("Could not fix missing sequence");
//		}
//
//		LOG_WARNING("Sequence " + fullInvalidKey + " does not exist, cloning from " + match);
//		// found a fix, so clone (with transforms)
//		// clone starting point
//		_info[fullInvalidKey] = _info[match];
//		_info[fullInvalidKey]["faked"] = "true"; // its fake now
//		_info[fullInvalidKey]["sequence"] = invalidKeyParts[1]; // set to proper sequence name
//
//		// set specifics, this is things nextSequences, etc.
//		if(regex_search(fullInvalidKey, regex("b$"))){
//			// nothing?
//		}
//		if(regex_search(fullInvalidKey, regex("a$"))){
//			// a key
//			// set nextSequence to proper loop
//			_info[fullInvalidKey]["nextSequence"] = invalidKeyParts[1] + "_loop";
//
//			// set victimResult
//			// seq01a -> seq02b
//			string resultSeqName = invalidKeyParts[1]; // seq01a
//			resultSeqName = createNextSequenceString(resultSeqName); // seq01a to seq02a
//			LOG_WARNING("MUST CONFIRM a_type sequence victimResult name is seq02b  (and not seq01b, probably isnt.)");
//			resultSeqName[resultSeqName.length()-1] = 'b'; // make seq02a into seq02b TODO: this might be wrong name
//			_info[fullInvalidKey]["victimResult"] = resultSeqName;
//		}
//		if(regex_search(fullInvalidKey, regex("_loop$"))){
//			// loop key
//			// set attackerResult
//			// set victimResult
//			LOG_WARNING("MUST CONFIRM loop to victim result sequence names (seq01a_loop to seq02b?)");
//			vector<string> seqnameParts;
//			boost::split(seqnameParts, invalidKeyParts[1], boost::is_any_of("_"));
//			string resultSeqName = seqnameParts[0]; // seq01a (ignores loop part which is seqnameParts[1])
//			resultSeqName = createNextSequenceString(resultSeqName); // seq01a to seq02a
//			_info[fullInvalidKey]["attackerResult"] = resultSeqName;
//			resultSeqName[resultSeqName.length()-1] = 'b'; // make seq02a into seq02b TODO: this might be wrong name
//			_info[fullInvalidKey]["victimResult"] = resultSeqName;
//			// set nextSequence to this sequence
//			_info[fullInvalidKey]["nextSequence"] = invalidKeyParts[1];
//		}
//
//		// clone transforms if keys exist
//		if(_info.find(match+":atk1") != _info.end()){
//			// clone atk1
//			_info[fullInvalidKey+":atk1"] = _info[match+":atk1"];
//			_info[fullInvalidKey+":atk1"]["faked"] = "true";
//			_info[fullInvalidKey+":atk1"]["sequence"] = invalidKeyParts[1];
//		}
//		if(_info.find(match+":atk2") != _info.end()){
//			// clone atk2
//			_info[fullInvalidKey+":atk2"] = _info[match+":atk2"];
//			_info[fullInvalidKey+":atk2"]["faked"] = "true";
//			_info[fullInvalidKey+":atk2"]["sequence"] = invalidKeyParts[1];
//		}
//		if(_info.find(match+":vic1") != _info.end()){
//			// clone vic1
//			_info[fullInvalidKey+":vic1"] = _info[match+":vic1"];
//			_info[fullInvalidKey+":vic1"]["faked"] = "true";
//			_info[fullInvalidKey+":vic1"]["sequence"] = invalidKeyParts[1];
//
//		}
//	}
//}
//
//
//
// Increments the sequence number in a sequence name
// eg: seq01a -> seq02a
//string SceneXMLBuilder::createNextSequenceString(string seq){
//	int seqNum;
//	char seqNumString[20];
//	char formatString[20];
//	string matchstring;
//
//
//	cmatch match;
//	string ret = seq;
//
//	// pull out the sequence number from the sequence name, so we can increment it
//	if(regex_match(seq.c_str(), match, regex("seq(\\d+)a"))){
//		matchstring = string(match[1].first, match[1].second);
//		sscanf(matchstring.c_str(), "%d", &seqNum); // find int
//		seqNum++; // inc int
//		// get correct width in format string
//		snprintf(formatString, sizeof(formatString), "%%0%dd", (int)matchstring.length());
//		snprintf(seqNumString, sizeof(seqNumString), formatString, seqNum);
//		ret.replace(ret.find_last_of(matchstring)-matchstring.length()+1,
//					matchstring.length(),
//					seqNumString);
//	}
//	else{
//		LOG_ERROR("Could not find int to increment");
//		abort();
//	}
//	return ret;
//}
//
