
#ifndef _jcUpdate_
#define _jcUpdate_

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MStatus.h>
#include <maya/MArgDatabase.h>
/*
	this cmd is used to update jcFeahter's texture
	to get fast interactively feedback in the viewport, you can make jcFeather not update texture in interactive mode
	jcFeather will update texture when rendering
*/
class jcUpdate : public MPxCommand
{
public:
        jcUpdate() {};
		~jcUpdate(){}; 

        MStatus  doIt( const MArgList& args );
        static void*    creator();
		static MSyntax newSyntax();

		void jcUpdateFeatherColor();
};

#endif