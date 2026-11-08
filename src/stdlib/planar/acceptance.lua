-- Complete accepted interpretation policy over sealed declarations, without source topology.
-- Normalization happens inside validation; preserve its complete successful values.
local function interpret_bundle(input)
  local result=validate_bundle(engine.object({bundle=input,capture=engine.object({status="not_captured"})}))
  if result.error then return result end
  local validated=result.value
  local bundle=validated.bundle
  local bundle_jobs={}
  for i,entry in ipairs(bundle.jobs) do bundle_jobs[i]=entry end
  bundle.jobs=sorted(bundle_jobs,function(entry)return entry.value.id end)
  local job_models={}
  for i,entry in ipairs(validated.jobs) do job_models[i]=entry end
  local jobs=sorted(job_models,function(job)return job.id end)
  return {value=engine.object({bundle=bundle,semantic=engine.object({dependencies=validated.dependencies,jobs=jobs})})}
end
