##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=Aligner
ConfigurationName      :=Release
WorkspacePath          := "/home/arne/Alligner Development"
ProjectPath            := "/home/arne/Alligner Development/Aligner"
IntermediateDirectory  :=./Release
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Arne Kutzner
Date                   :=12/09/13
CodeLitePath           :="/home/arne/.codelite"
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="Aligner.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)../header 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)rt 
ArLibs                 :=  "rt" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -O2 -std=c++0x -msse4.1 -Wall -Wno-unused -Wno-reorder  $(Preprocessors)
CFLAGS   :=  -O2 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/src_fasta$(ObjectSuffix) $(IntermediateDirectory)/src_support$(ObjectSuffix) $(IntermediateDirectory)/src_lib$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@test -d ./Release || $(MakeDirCommand) ./Release

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_fasta$(ObjectSuffix): ../src/fasta.c $(IntermediateDirectory)/src_fasta$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/arne/Alligner Development/src/fasta.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_fasta$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_fasta$(DependSuffix): ../src/fasta.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_fasta$(ObjectSuffix) -MF$(IntermediateDirectory)/src_fasta$(DependSuffix) -MM "../src/fasta.c"

$(IntermediateDirectory)/src_fasta$(PreprocessSuffix): ../src/fasta.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_fasta$(PreprocessSuffix) "../src/fasta.c"

$(IntermediateDirectory)/src_support$(ObjectSuffix): ../src/support.c $(IntermediateDirectory)/src_support$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/arne/Alligner Development/src/support.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_support$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_support$(DependSuffix): ../src/support.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_support$(ObjectSuffix) -MF$(IntermediateDirectory)/src_support$(DependSuffix) -MM "../src/support.c"

$(IntermediateDirectory)/src_support$(PreprocessSuffix): ../src/support.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_support$(PreprocessSuffix) "../src/support.c"

$(IntermediateDirectory)/src_lib$(ObjectSuffix): ../src/lib.cpp $(IntermediateDirectory)/src_lib$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/arne/Alligner Development/src/lib.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_lib$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_lib$(DependSuffix): ../src/lib.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_lib$(ObjectSuffix) -MF$(IntermediateDirectory)/src_lib$(DependSuffix) -MM "../src/lib.cpp"

$(IntermediateDirectory)/src_lib$(PreprocessSuffix): ../src/lib.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_lib$(PreprocessSuffix) "../src/lib.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/src_fasta$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_fasta$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_fasta$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_support$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_support$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_support$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_lib$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_lib$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_lib$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) "../.build-release/Aligner"


