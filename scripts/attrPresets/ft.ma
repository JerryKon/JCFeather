//Maya ASCII 2011 scene
//Name: ft.ma
//Last modified: Mon, Sep 02, 2013 21:49:22 PM
//Codeset: 936
requires maya "2011";
requires "stereoCamera" "10.0";
currentUnit -l centimeter -a degree -t film;
fileInfo "application" "maya";
fileInfo "product" "Maya 2011";
fileInfo "version" "2011 x64";
fileInfo "cutIdentifier" "001200000000-796618";
fileInfo "osv" "Microsoft Windows 7 Ultimate Edition, 64-bit Windows 7 Service Pack 1 (Build 7601)\n";
createNode transform -n "featherTemplate";
createNode mesh -n "featherTemplateShape" -p "featherTemplate";
	setAttr -k off ".v";
	setAttr ".mb" no;
	setAttr ".csh" no;
	setAttr ".rcsh" no;
	setAttr ".vis" no;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr -s 13 ".uvst[0].uvsp[0:12]" -type "float2" 0.76245809 0.081053138
		 0.91933846 0.32738674 0.49999994 0.25366646 0.49999994 0.0073328614 0.91933846 0.57372034
		 0.49999988 0.5 0.80364609 0.82005382 0.49999988 0.7463336 0.49999988 0.9926672 0.080661356
		 0.32738674 0.23754179 0.081053138 0.080661297 0.57372034 0.19635373 0.82005382;
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcol" yes;
	setAttr ".dcc" -type "string" "Diffuse";
	setAttr ".smo" no;
	setAttr ".covm[0]"  0 1 1;
	setAttr ".cdvm[0]"  0 1 1;
	setAttr ".bnr" 0;
	setAttr -s 13 ".vt[0:12]"  0.14963508 -1.110223e-016 0.31294298 0.64963508 -1.110223e-016 0.5
		 1.14963508 -1.110223e-016 0.5 1.64963508 -1.110223e-016 0.36205366 2 0 0 0 0 0 0.5 0 0
		 1 0 0 1.5 0 0 0.14963508 1.110223e-016 -0.31294298 0.64963508 1.110223e-016 -0.5
		 1.14963508 1.110223e-016 -0.5 1.64963508 1.110223e-016 -0.36205366;
	setAttr -s 20 ".ed[0:19]"  0 1 0 0 5 0 1 2 0 1 6 1 2 3 0 2 7 1 3 4 0
		 3 8 1 5 6 1 5 9 0 6 7 1 6 10 1 7 8 1 7 11 1 8 4 1 8 12 1 9 10 0 10 11 0 11 12 0 12 4 0;
	setAttr -s 8 ".fc[0:7]" -type "polyFaces" 
		f 4 0 3 -9 -2
		mu 0 4 0 1 2 3
		f 4 2 5 -11 -4
		mu 0 4 1 4 5 2
		f 4 4 7 -13 -6
		mu 0 4 4 6 7 5
		f 3 6 -15 -8
		mu 0 3 6 8 7
		f 4 8 11 -17 -10
		mu 0 4 3 2 9 10
		f 4 10 13 -18 -12
		mu 0 4 2 5 11 9
		f 4 12 15 -19 -14
		mu 0 4 5 7 12 11
		f 3 14 -20 -16
		mu 0 3 7 8 12;
	setAttr ".cd" -type "dataPolyComponent" Index_Data Edge 0 ;
	setAttr ".cvd" -type "dataPolyComponent" Index_Data Vertex 0 ;
	setAttr ".hfd" -type "dataPolyComponent" Index_Data Face 0 ;
	setAttr ".bw" 3;
	setAttr ".difs" yes;
select -ne :time1;
	setAttr ".o" 1;
	setAttr ".unw" 1;
select -ne :renderPartition;
	setAttr -s 2 ".st";
select -ne :initialShadingGroup;
	setAttr ".ro" yes;
select -ne :initialParticleSE;
	setAttr ".ro" yes;
select -ne :defaultShaderList1;
	setAttr -s 2 ".s";
select -ne :postProcessList1;
	setAttr -s 2 ".p";
select -ne :defaultRenderingList1;
select -ne :renderGlobalsList1;
select -ne :defaultResolution;
	setAttr ".pa" 1;
select -ne :hardwareRenderGlobals;
	setAttr ".ctrs" 256;
	setAttr ".btrs" 512;
select -ne :defaultHardwareRenderGlobals;
	setAttr ".fn" -type "string" "im";
	setAttr ".res" -type "string" "ntsc_4d 646 485 1.333";
connectAttr "featherTemplateShape.iog" ":initialShadingGroup.dsm" -na;
// End of ft.ma
