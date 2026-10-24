-- Product-owned export table; preceding helpers are fixed by the build.
-- No runtime require, paths, or executable job input.
local function parse_json(input)
  local ctx=schema_context({},false)
  local request=ctx.normalize(input,record({file=req(PATH),bytes=req(primitive("raw"))}),"","json")
  if #ctx.errors>0 then return ctx.finish({}) end
  input=request
  local file=logical_file(input.file)
  if file=="" then
    return {error={code="path_denied",message="Invalid logical JSON path",file="",field="file",details={}}}
  end
  local ok,result=pcall(json_parse,input.bytes,file)
  if ok then return {value=result} end
  if type(result)=="table" then return {error=result} end
  error(result,0)
end
return {
  copy = function(input) return input end,
  hash_value = function(input) return engine.hash_value(input.domain, input.value) end,
  file_digest = function(input) return engine.file_digest(input) end,
  fragment_digest = function(input) return engine.fragment_digest(input) end,
  parse_json = parse_json,
  route_workspace = route_workspace,
  normalize_bundle = normalize_bundle,
  validate_bundle = validate_bundle,
  shape_prepare = function(input) return prepare_shapes(input.validation,input.context) end,
  shape_check = check_shape,
  shape_finish = finish_shapes,
  interpret_bundle = interpret_bundle,
  select_role = select_role,
  select_profile_material = select_profile_material,
  render = render,
  assemble_projection_material = assemble_projection_material,
  prepare_profile_render = prepare_profile_render,
  render_profile_material = render_profile_material,
  role_profile_material = role_profile_material,
  role_projection_base_material = role_projection_base_material,
}
