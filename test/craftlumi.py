import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")
process.load("CondCore.DBCommon.CondDBCommon_cfi")
#process.CondDBCommon.connect = 'sqlite_file:offlinelumi.db'
process.CondDBCommon.connect = 'oracle://cms_orcon_prod/CMS_COND_31X_RUN_INFO'
process.CondDBCommon.DBParameters.authenticationPath = '/nfshome0/xiezhen'

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.CondDBCommon,
    BlobStreamerName = cms.untracked.string('TBufferBlobStreamingService'),
    timetype = cms.untracked.string('lumiid'),
    logconnect = cms.untracked.string('oracle://cms_orcon_prod/CMS_COND_31X_POPCONLOG'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string('LuminosityInfoRcd'),
        tag = cms.string('lumicraft09_v1')
    ))
)

process.source = cms.Source("EmptyIOVSource",
    lastValue = cms.uint64(1),
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(1),
    interval = cms.uint64(1)
)

process.lumio2o = cms.EDAnalyzer("LuminosityPopConAnalyzer",
    Source = cms.PSet(
      lumiReaderName = cms.string('omds'),
      startRun = cms.int32(109049),
      numberOfRuns = cms.untracked.int32(1),
      connect = cms.string('oracle://cms_omds_lb/CMS_LUMI'),
      lumiVersionNumber = cms.int32(1),
      authenticationPath = cms.string('/nfshome0/xiezhen')
    ),
    SinceAppendMode = cms.bool(True),
    name = cms.untracked.string('LuminosityInfo'),
    record = cms.string('LuminosityInfoRcd'),
    loggingOn = cms.untracked.bool(True),
    debug = cms.bool(True)
)

process.p = cms.Path(process.lumio2o)