import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")
process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.CondDBCommon.connect = '##destdb##'
process.CondDBCommon.DBParameters.authenticationPath = '/nfshome0/xiezhen'

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.CondDBCommon,
    BlobStreamerName = cms.untracked.string('TBufferBlobStreamingService'),
    timetype = cms.untracked.string('lumiid'),
    logconnect = cms.untracked.string('##logdb##'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string('LuminosityInfoRcd'),
        tag = cms.string('##tagname##')
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
      startRun = cms.int32(##startrun##),
      numberOfRuns = cms.untracked.int32(##numberofrun##),
      connect = cms.string('oracle://cms_omds_lb/CMS_LUMI'),
      lumiVersionNumber = cms.int32(##lumiversion##),
      authenticationPath = cms.string('/nfshome0/xiezhen')
    ),
    SinceAppendMode = cms.bool(True),
    name = cms.untracked.string('LuminosityInfo'),
    record = cms.string('LuminosityInfoRcd'),
    loggingOn = cms.untracked.bool(False),
    debug = cms.bool(True)
)

process.p = cms.Path(process.lumio2o)