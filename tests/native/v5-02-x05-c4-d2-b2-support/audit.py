"""Original retained authority source boundary; behavioral proof is separate."""
from pathlib import Path
import json,sys,re
R=Path(__file__).resolve().parents[3]
a=(R/'src/engine/capture/admission.cpp').read_text();n=(R/'src/engine/capture/native.cpp').read_text();p=(R/'src/engine/capture/profile.cpp').read_text()
i=(R/'src/engine/capture/admission.cppm').read_text()
assert 'std::shared_ptr<implementation> _impl' in i and 'std::shared_ptr<capture_admission::implementation> _state' in i
assert 'std::size_t _index' in i or 'std::size_t                                     _index' in i or '_index;' in i
retain=a[a.index('capture_admission::retain_file'):a.index('retained_file_receipt::retained_file_receipt')]
assert 'implementation::kind::supplied_file' in retain and '_impl->check()' in retain
assert 'sizeof(retained_file_receipt) + sizeof(std::optional<retained_file_receipt>)' in retain
read=n[n.index('  retained read_file('):n.index('  void retain(')]
assert read.index('authority->verify(f.fd)')<read.index('std::string data(')<read.index('::read(')
assert 'original->same_selection(*authority)' in read and 'admission->poison()' in read
assert 'std::optional<retained_file_receipt> authority' in n or 'authority;' in n
assert 'fresh.authority.emplace(std::move(receipt))' in n
assert 'Retained file authority cannot be replaced' in n
assert 'implementation::operation guard' in n and re.search(r'operation\s+guard\{this\}',n)
assert '_impl->admission.retain_file(slot)' in p and 'std::move(*token)' in p
snapshot=p[p.index('profile_capture::snapshot()'):p.index('profile_capture::admit_root')]
assert '.healthy()' in snapshot and 'verify(' not in snapshot
for text in [a,n,p]:
 for forbidden in ['lexically_normal(', 'lua_']: assert forbidden not in text
result=dict(shared_original_state=True,immutable_selection=True,supplied_files_only=True,actual_fd_before_content=True,stored_authority_on_rereads=True,no_cache_authority_replacement=True,no_filesystem_terminal_observer=True,private_profile_forwarding=True)
Path(sys.argv[1]).write_text(json.dumps(result,sort_keys=True,indent=2)+'\n')
assert 'r.pending->verify(leaf.fd)' in n
assert 'entry.authority.emplace(std::move(*r.pending))' in n
assert 'session.add_input(std::move(logical), std::move(file), std::move(*token))' in p
bound=n[n.index('native_capture_session::add_input(std::string logical, std::string file,'):n.index('native_capture_session::read(std::string_view root, std::string_view path, bool text)')]
assert bound.index('receipt.verify(leaf.fd)')<bound.index('_impl->roots.reserve(')<bound.index('_impl->roots.push_back(')
assert 'open_root(parent, true)' in bound


root=a[a.index('retained_root_receipt::discover'):]
assert 'valid_path(path)' in root and 'metadata_flags(directory)' in root
assert '::read(' not in root and 'opendir' not in root and 'readdir' not in root
assert 'std::to_string(_state->selections.size())' in root and 'kind::discovered_file' in root
assert '_state->discovered.find(key)' in root and '_state->discovered.emplace' in root
assert 'selected.type == capture_admission::implementation::kind::supplied_file' in root
assert '_state->charge(' in root and 'equal_folded(prefix, old_prefix)' in root
assert 'std::move(discovered)' in read and 'r.authority->verify(r.fd.fd)' in read
verify=n[n.index('  void verify('):n.index('native_capture_session::native_capture_session')]
assert verify.index('found != expected') < verify.index('auto fresh')
assert 'r.authority->verify(reopened.fd)' in verify
assert 'Discovery after initial capture' in read
c=(R/'src/engine/capture/capture.cpp').read_text(); ci=(R/'src/engine/capture/capture.cppm').read_text()
assert 'retained_root_receipt receipt' in c and 'std::move(receipt)' in c
assert 'import planar.engine.capture.profile' not in ci and 'import planar.engine.runtime' not in ci
result.update(root_original_state=True,portable_descriptor_discovery=True,charged_index=True,bounded_private_ordinal_keys=True,explicit_file_selection_precedence=True,root_checks_before_reads_and_enumeration=True,final_membership_before_rereads=True,no_final_discovery=True,capture_wrapper_no_profile_import=True)
Path(sys.argv[1]).write_text(json.dumps(result,sort_keys=True,indent=2)+'\n')
print('D2 B2 original authority source boundary audit pass')

file_lookup=a[a.index('capture_admission::verify_file'):a.index('capture_admission::verify_root')]
assert 's.type != implementation::kind::discovered_file' in file_lookup
root_lookup=a[a.index('capture_admission::verify_root'):a.index('bool capture_admission::healthy')]
assert 's.type == implementation::kind::supplied_root' in root_lookup
file_factory=a[a.index('capture_admission::retain_file'):a.index('retained_file_receipt::retained_file_receipt')]
assert 'selected.type == implementation::kind::supplied_file' in file_factory
root_factory=a[a.index('capture_admission::retain_root'):a.index('retained_root_receipt::retained_root_receipt')]
assert '_impl->selections[i].type == implementation::kind::supplied_root' in root_factory
assert 's.type != kind::discovered_file && s.slot == slot' in a
