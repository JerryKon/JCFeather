
#ifndef _jcFeatherAbout_
#define _jcFeatherAbout_

#include <maya/MPxCommand.h>

class jcFeatherAbout : public MPxCommand
{
public:
        jcFeatherAbout() {};
		~jcFeatherAbout(){}; 

        MStatus  doIt( const MArgList& args );
        static void*    creator();
};

#endif