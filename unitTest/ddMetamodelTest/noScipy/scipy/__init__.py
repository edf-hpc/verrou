# Fake scipy package (PYTHONPATH=noScipy) : test of verrou_dd_metamodel without scipy
raise ImportError("scipy hidden by unitTest/ddMetamodelTest/noScipy")
