import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")
process.load("CondCore.DBCommon.CondDBCommon_cfi")
#process.CondDBCommon.connect = 'sqlite_file:offlinelumi.db'
process.CondDBCommon.connect = 'oracle://cms_orcon_prod/CMS_COND_31X_RUN_INFO'
process.CondDBCommon.DBParameters.authenticationPath = '/nfshome0/xiezhen'
process.CondDBCommon.DBParameters.messageLevel = 0

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.CondDBCommon,
    BlobStreamerName = cms.untracked.string('TBufferBlobStreamingService'),
    timetype = cms.untracked.string('lumiid'),
    logconnect = cms.untracked.string('oracle://cms_orcon_prod/CMS_COND_31X_POPCONLOG'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string('HLTScalerInfoRcd'),
        tag = cms.string('hltinfocraft09')
    ))
)

process.source = cms.Source("EmptyIOVSource",
    lastValue = cms.uint64(1),
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(1),
    interval = cms.uint64(1)
)

process.hltscalero2o = cms.EDAnalyzer("HLTScalerPopConAnalyzer",
    Source = cms.PSet(
      hltscalerReaderName = cms.string('db'),
      startRun = cms.int32(108239),
      #numberOfRuns = cms.untracked.int32(1),
      endRun = cms.untracked.int32(109049),
      connect = cms.string('oracle://cms_rcms/CMS_RUNINFO'),
      authenticationPath = cms.string('/nfshome0/xiezhen'),
      messageLevel = cms.untracked.int32(0)
    ),
    SinceAppendMode = cms.bool(True),
    name = cms.untracked.string('HLTScalerInfo'),
    record = cms.string('HLTScalerInfoRcd'),
    loggingOn = cms.untracked.bool(True),
    debug = cms.bool(True)
)

process.p = cms.Path(process.hltscalero2o)