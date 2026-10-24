"""Reconstruct pre-edit owned files; verify approved B3-C frozen identities."""
from pathlib import Path
import hashlib,json
root=Path(__file__).resolve().parents[3];e=root/'tests/native/v5-02-x05-b4-a-evidence';old=json.loads((root/'tests/native/v5-02-x05-b3-c-evidence/frozen-source.json').read_text());paths=['src/stdlib/planar/render.lua','src/stdlib/planar/bootstrap.lua','src/engine/runtime/external_registration.t.cpp','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md'];rows={}
for name in paths:
 s=(root/name).read_text()
 if name.endswith('render.lua'):
  body=s[s.index('  local observations='):s.index('\nlocal function render(input)')]
  body=body.replace('  if shape then\n    section("shape_policy",presentation_data("shape guidance; trusted policy",shape.guidance)..presentation_data("shape obligations; canonical JSON",presentation_json(W.shape.obligations)),true)\n  end\n','')
  a=body.index('  local rendered_prerequisites=');b=body.index('  local manifest=',a);body=body[:a]+body[b:]
  body=body.replace('prerequisites=rendered_prerequisites,registry=','prerequisites=selected.prerequisites,registry=')
  a=body.index('  if shape then\n    local identity=');b=body.index('  return ctx.finish(engine.object({version=1,kind="render"',a);body=body[:a]+body[b:]
  prefix=s[s.index('local function render(input)'):s.index('  return render_material(ctx,request,selected,model)')]
  s='-- Pure projection over approved selection and retained bytes. No fresh reads.\n'+prefix+body
 elif name.endswith('bootstrap.lua'):s=s.replace('  render_profile_material = render_profile_material,\n','')
 elif name.endswith('external_registration.t.cpp'):s=s[:s.index('\nnamespace {\n// Pure helper composition only:')]
 data=s.encode();digest=hashlib.sha256(data).hexdigest()
 assert digest==old[name],(name,digest,old[name])
 target=e/'baseline'/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data);rows[name]=digest
(e/'baseline.json').write_text(json.dumps({'method':'Post-edit reconstruction by exact inverse transformation, checked against approved B3-C frozen SHA256; unchanged prose copied before edits. Not a pre-edit capture.','sha256':rows},indent=2)+'\n')
