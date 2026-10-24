local D={id='custom',abi='shape_v1',revision='final',stages='optional',help='shape'}
registration.shape(D,function(input)
  return engine.object{ok=true,value=engine.object{
    guidance=input.job.notes and input.job.notes.bytes or 'none',
    obligations=engine.array{
      engine.object{id='z',producer='library',consumer='command',phase='complete',quality='done'},
      engine.object{id='a',producer='library',consumer='command',phase='start',quality='done'}
    }
  }}
end)
