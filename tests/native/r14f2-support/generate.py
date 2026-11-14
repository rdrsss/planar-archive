"""Exact reversible shared response variant: refuse encoding or throw after native publication."""
from pathlib import Path
import hashlib,json,sys
source,output=map(Path,sys.argv[1:]);output.mkdir(parents=True,exist_ok=True)
manifest={}
for name in ['invocation.cppm','invocation.cpp']:
 original=(source/name).read_text()
 substitutions=[('planar.cmd.invocation;', 'planar.test.export_command;'),
                ('namespace planar::cmd {','namespace planar::cmd::handlers_fault {')]
 if name=='invocation.cpp':
  substitutions += [('    export_published = true;', '    export_published = true;\n    if (options.target.ends_with("service-facts")) throw std::bad_alloc();'),
    ('    auto result = vm->invoke(std::move(options), *services);', '    const bool allocation_fault=std::holds_alternative<engine::command_export_options>(options) && std::get<engine::command_export_options>(options).target.ends_with("allocation");\n    auto result = vm->invoke(std::move(options), *services);\n    if (allocation_fault && published()) throw std::bad_alloc();'),
    ('engine::codec_encode(envelope, {16777215, 256, 4000000})','engine::codec_encode(envelope, {1, 256, 4000000})')]
 generated=original;counts=[]
 for before,after in substitutions:
  count=generated.count(before);assert count==1,(name,before,count)
  counts.append(count);generated=generated.replace(before,after)
 restored=generated
 for before,after in reversed(substitutions):restored=restored.replace(after,before)
 assert restored==original
 dest=output/name
 if not dest.exists() or dest.read_text()!=generated:dest.write_text(generated)
 manifest[name]=dict(source_sha256=hashlib.sha256(original.encode()).hexdigest(),generated_sha256=hashlib.sha256(generated.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=b,after=a,count=c) for (b,a),c in zip(substitutions,counts)])
_t=json.dumps(manifest,indent=2)+'\n'
if not (output/'generation.json').exists() or (output/'generation.json').read_text()!=_t:(output/'generation.json').write_text(_t)
