import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")
process.PoolDBESSource = cms.ESSource("PoolDBESSource",
    DBParameters = cms.PSet(
        messageLevel = cms.untracked.int32(0),
        authenticationPath = cms.untracked.string('/afs/cern.ch/cms/DB/conddb')
    ),
    toGet = cms.VPSet(cms.PSet(
        record = cms.string('LuminosityInfoRcd'),
        tag = cms.string('lumi83037_v1')
    )),
    connect = cms.string('sqlite_file:offlinelumi.db'),
    #connect = cms.string('oracle://cms_orcoff_prep/CMS_COND_RUN_INFO'),                                  
    BlobStreamerName = cms.untracked.string('TBufferBlobStreamingService')                          
)
 
process.maxEvents = cms.untracked.PSet(
     input = cms.untracked.int32(21)
)
process.source = cms.Source("EmptySource",
     numberEventsInRun = cms.untracked.uint32(21),
     firstRun = cms.untracked.uint32(83037),
     numberEventsInLuminosityBlock = cms.untracked.uint32(1),
     firstLuminosityBlock = cms.untracked.uint32(1)
)

#process.source = cms.Source("EmptyIOVSource",
#    timetype = cms.string('lumiid'),
#    firstValue = cms.uint64(356641199357953),
#    lastValue = cms.uint64(356641199357973),
#    interval = cms.uint64(1)
#)

process.prod = cms.EDAnalyzer("LuminosityInfoAnalyzer")

process.p = cms.Path(process.prod)

