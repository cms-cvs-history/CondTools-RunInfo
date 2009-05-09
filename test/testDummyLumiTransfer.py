import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")
process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.CondDBCommon.connect = 'sqlite_file:offlinelumi.db'

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.CondDBCommon,
    BlobStreamerName = cms.untracked.string('TBufferBlobStreamingService'),
    timetype = cms.untracked.string('lumiid'),
    logconnect = cms.untracked.string('sqlite_file:log.db'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string('LuminosityInfoRcd'),
        tag = cms.string('lumitest')
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
      lumiReaderName = cms.string('dummy'),
      startRun = cms.int32(10),
      numberOfRuns = cms.untracked.int32(2),
      lumiVersionNumber = cms.int32(1)
    ),
    SinceAppendMode = cms.bool(True),
    name = cms.untracked.string('LuminosityInfo'),
    record = cms.string('LuminosityInfoRcd'),
    loggingOn = cms.untracked.bool(False),
    debug = cms.bool(False)
)

process.p = cms.Path(process.lumio2o)
