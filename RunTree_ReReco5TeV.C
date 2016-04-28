// Load DatasetManager in ROOT 6
R__LOAD_LIBRARY(DatasetManager/DatasetManager.C+)
#include "DatasetManager/DatasetManager.h"

/********************************************************
 * Main function
 ********************************************************/
void RunTree_ReReco5TeV(TString  sampleName     = "TTbar_Madgraph",
			Int_t    nSlots         = 1,
			Bool_t   DoSystStudies  = false,
			Long64_t nEvents        = 0,
			Bool_t   G_CreateTree   = false,
			Int_t    stopMass       = 0,
			Int_t    lspMass        = 0,
                        Float_t  SusyWeight     = 0.0) {
  
  // VARIABLES TO BE USED AS PARAMETERS...
  Float_t G_Total_Lumi    = 19664.225;   
  Float_t G_Event_Weight  = 1.0;         
  Bool_t  G_IsData        = false;       
  Float_t G_LumiForPUData = 19468.3;     // luminosity in http://www.hep.uniovi.es/jfernan/PUhistos
  Bool_t  DoSF            = true;
  Bool_t  DoDF            = true;
  Bool_t  G_IsMCatNLO     = false;

  // PAF mode
  //----------------------------------------------------------------------------
  cout << endl; 
  PAFIExecutionEnvironment* pafmode = 0;
  if (nSlots <=1 ) {
    PAF_INFO("RunTree_ReReco5TeV", "Sequential mode chosen");
    pafmode = new PAFSequentialEnvironment();
  }
  else if (nSlots <=8) {
    PAF_INFO("RunTree_ReReco5TeV", "PROOF Lite mode chosen");
    pafmode = new PAFPROOFLiteEnvironment(nSlots);
  }
  else {
    PAF_INFO("RunTree_ReReco5TeV", "PoD mode chosen");
    pafmode = new PAFPoDEnvironment(nSlots);
  }

  // Create PAF Project whith that environment
  //----------------------------------------------------------------------------
  PAFProject* myProject = new PAFProject(pafmode);

  // Base path to input files
  //----------------------------------------------------------------------------
  TString dataPath = "/pool/ciencias/TreesDR76X/5TeV/v1";

  // INPUT DATA SAMPLE
  //----------------------------------------------------------------------------
  TString userhome = "/mnt_pool/fanae105/user/$USER/";
  DatasetManager* dm = DatasetManager::GetInstance();
  dm->SetTab("5TeVDR76X25nsAOD");
  //dm->RedownloadFiles();

  // Deal with data samples
  if ((sampleName == "DoubleEG"   ||
       sampleName == "DoubleMuon" ||
       sampleName == "MuonEG"     ||
       sampleName == "SingleEle"  ||
       sampleName == "SingleMu")) {
    cout << "   + Data..." << endl;
    
    TString datasuffix[] = {
      "Run2015C_16Dec",
      "Run2015D_16Dec"
    };
    const unsigned int nDataSamples = 3;
    for(unsigned int i = 0; i < nDataSamples; i++) {
      TString asample = Form("Tree_%s_%s",sampleName.Data(), datasuffix[i].Data());
      cout << "   + Looking for " << asample << " trees..." << endl;
      myProject->AddDataFiles(dm->GetRealDataFiles(asample));
    }
    G_Event_Weight = 1.;
    G_IsData = true;
  }
  else{ // Deal with MC samples
    G_IsData = false;
    dm->LoadDataset(sampleName);
    if(sampleName != "Test")   myProject->AddDataFiles(dm->GetFiles());
    if(sampleName.Contains("aMCatNLO") || sampleName.Contains("amcatnlo") ){
      G_Event_Weight = dm->GetCrossSection() / dm->GetSumWeights();

      cout << endl;
      cout << " weightSum(MC@NLO) = " << dm->GetSumWeights()     << endl;
    }
    else if(sampleName == "Test"){
      TString localpath="/pool/ciencias/users/user/palencia/";
      TString sample = "treeTtbar_jan19.root";
      myProject->AddDataFile(localpath + sample);
      G_Event_Weight = 1;
    }
    else {	 
      G_Event_Weight = dm->GetCrossSection() / dm->GetEventsInTheSample();
    }
    
    if(nEvents == 0) nEvents = dm->GetEventsInTheSample();

    cout << endl;
    cout << " #==============================================="   << endl;
    cout << " #      sampleName = " << sampleName		  << endl;
    cout << " #       x-section = " << dm->GetCrossSection()	  << endl;
    cout << " #      	nevents = " << dm->GetEventsInTheSample() << endl;
    cout << " #  base file name = " << dm->GetBaseFileName()	  << endl;
    cout << " #      	 weight = " << G_Event_Weight		  << endl;
    cout << " #      	 isData = " << G_IsData 		  << endl;
    cout << " #==============================================="   << endl;
    cout << endl;
  }
  
	// Output file name
  //----------------------------------------------------------------------------
  Bool_t G_Use_CSVM = true;
  TString outputDir = "./temp";
  if(sampleName.BeginsWith("T2tt")) outputDir += "/Susy";

  gSystem->mkdir(outputDir, kTRUE);

  std::ostringstream oss;      
  oss << G_Total_Lumi;

  TString LumiString = oss.str();
  TString outputFile = outputDir;
  outputFile += "/Tree_" + sampleName + ".root";

  PAF_INFO("RunTree_ReReco5TeV", Form("Output file = %s", outputFile.Data()));
  myProject->SetOutputFile(outputFile);

  if(sampleName.Contains("aMCatNLO") || sampleName.Contains("amcatnlo") ){
    PAF_INFO("RunTree_ReReco5TeV", "This is a MC@NLO sample!");
    G_IsMCatNLO = true;
  }

  // Parameters for the analysis
  //----------------------------------------------------------------------------
  myProject->SetInputParam("sampleName",    sampleName       );
  myProject->SetInputParam("IsData",        G_IsData         );
  myProject->SetInputParam("UseCSVM",       G_Use_CSVM       );
  myProject->SetInputParam("weight",        G_Event_Weight   );
  myProject->SetInputParam("LumiForPU",     G_LumiForPUData  );
  myProject->SetInputParam("TotalLumi",     G_Total_Lumi     );
  myProject->SetInputParam("DoSystStudies", DoSystStudies    );
  myProject->SetInputParam("DoSF"         , DoSF             );
  myProject->SetInputParam("DoDF"         , DoDF             );
  myProject->SetInputParam("stopMass"     , stopMass         );
  myProject->SetInputParam("lspMass"      , lspMass          );
  myProject->SetInputParam("IsMCatNLO"    , G_IsMCatNLO      );  
  myProject->SetInputParam("CreateTree"   , G_CreateTree     );

 
  if(nEvents != 0) myProject->SetNEvents(nEvents);

  // Name of analysis class
  //----------------------------------------------------------------------------
  myProject->AddSelectorPackage("TOP5TeVAnalyzer");

  // Additional packages
  //----------------------------------------------------------------------------
  myProject->AddPackage("mt2");
  myProject->AddPackage("PUWeight");
  myProject->AddPackage("BTagSFUtil");
  myProject->AddPackage("LeptonSF");


  // Let's rock!
  //----------------------------------------------------------------------------
  myProject->Run();
}