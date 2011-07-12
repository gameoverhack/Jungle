/*
 *  VectorLoader.h
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

template <class vectorType>
bool loadVector(string filePath, vector< vectorType > * vec) {
	LOGGER->log(JU_LOG_VERBOSE, "Vectorloader", "Load vector:: " + filePath);
	std::ifstream ifs(filePath.c_str());
	if(ifs.fail()){
		LOGGER->log(JU_LOG_ERROR, "VectorLoader", "Could not load vector: " + filePath);
		abort(); // Could be a bit over zealous
	}
    boost::archive::text_iarchive ia(ifs);
    ia >> (*vec);
	return true;
}
