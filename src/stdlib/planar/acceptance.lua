-- Complete accepted interpretation policy over sealed declarations, without source topology.
-- Normalization happens inside validation; preserve its complete successful values.
local function interpret_bundle(input)
  local result=validate_bundle(engine.object({bundle=input,capture=engine.object({status="not_captured"})}))
  if result.error then return result end
  local validated=result.value
  local bundle=validated.bundle
  bundle.jobs=sorted(bundle.jobs,function(entry)return entry.value.id end)
  local jobs=sorted(validated.jobs,function(job)return job.id end)
  return {value=engine.object({bundle=bundle,semantic=engine.object({dependencies=validated.dependencies,jobs=jobs})})}
end
