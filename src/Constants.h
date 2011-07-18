/*
 *  constants.h
 *  Jungle
 *
 *  Created by ollie on 4/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_CONSTANTS
#define _H_CONSTANTS

const string kLAST_SEQUENCE_TOKEN = "__FINAL_SEQUENCE__";
const int kNoUserAction = 0;
const int kAttackerAction = 1;
const int kVictimAction = 2;

enum AppControllerState{
	kAPPCONTROLLER_INIT,
	kAPPCONTROLLER_LOADING,
	kAPPCONTROLLER_RUNNING,
	kAPPCONTROLLER_EXITING
};

enum DataControllerState{
	kDATACONTROLLER_INIT,
	kDATACONTROLLER_SCENE_PARSING,
	kDATACONTROLLER_SCENE_ANALYSING,
	kDATACONTROLLER_FINISHED
};

enum SceneXMLParserState{
	kSCENEXMLPARSER_INIT,
	kSCENEXMLPARSER_SETUP,
	kSCENEXMLPARSER_PARSE_XML,
	kSCENEXMLPARSER_VALIDATING_MOVIE_FILE_EXISTENCE,
	kSCENEXMLPARSER_VALIDATING_FILE_METADATA,
	kSCENEXMLPARSER_VALIDATING_MOVIE_TRANSFORM_LENGTHS,
	kSCENEXMLPARSER_CREATING_APPMODEL,
	kSCENEXMLPARSER_FINISHED
};	

#endif