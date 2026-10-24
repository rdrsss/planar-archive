#include "<repo>/src/kernel/runtime/schema.t.cpp"

namespace {
std::vector<std::string> id_list(value_ptr v, const std::string& key="id") {
 std::vector<std::string> out; for(auto& x:v->items) out.push_back(x->fields.at(key)->string); return out;
}
value_ptr question(std::string id,value_ptr scope,std::string blocks,bool answered=false) {
 auto q=record_value({{"id",string_value(id)},{"scope",scope},{"prompt",md("Clarify this constraint")},{"blocks",string_value(blocks)}});
 if(answered)q=changed(q,"answer",md("Agreed exact answer"));return q;
}
}
TEST_CASE("review scoped arrays blocking questions and accepted obligations", "[reviewer]") {
 fixture f;
 auto b=spec_field(f.bundle,"questions",array_value({question("globalq",string_value("global"),"start"),question("libraryq",strings({"library"}),"complete",true),question("bothq",strings({"library","command"}),"none")}));
 for(auto role:{"library_worker","cli_worker","reviewer"}) {
  auto r=selection_request(f,"split",role);r=selected_bundle(r,b);auto out=selected(r);auto W=out->fields.at("work")->fields.at("value");
  bool command=std::string_view(role)=="cli_worker",lib=std::string_view(role)=="library_worker";
  CHECK(id_list(W->fields.at("units"))==(command?std::vector<std::string>{"command"}:lib?std::vector<std::string>{"library"}:std::vector<std::string>{"command","library"}));
  CHECK(id_list(W->fields.at("milestones"))==(command?std::vector<std::string>{"cli"}:lib?std::vector<std::string>{"api"}:std::vector<std::string>{"api","cli"}));
  CHECK(id_list(W->fields.at("prescriptions"))==(command?std::vector<std::string>{"command_rule"}:lib?std::vector<std::string>{"library_rule"}:std::vector<std::string>{"command_rule","library_rule"}));
  CHECK(id_list(W->fields.at("dependencies"))==(lib?std::vector<std::string>{}:std::vector<std::string>{"develop","integrate"}));
  CHECK(id_list(W->fields.at("facets"))==(lib?std::vector<std::string>{}:std::vector<std::string>{"library_files"}));
  CHECK(id_list(W->fields.at("questions"))==(command?std::vector<std::string>{"bothq","globalq"}:std::vector<std::string>{"bothq","globalq","libraryq"}));
  exact_value(item_named(W->fields.at("questions"),"bothq")->fields.at("scope"),command?strings({"command"}):lib?strings({"library"}):strings({"command","library"}));
  CHECK(item_named(out->fields.at("prerequisites"),"question:globalq")->fields.at("status")->string=="unavailable");
  if(!command)CHECK(item_named(out->fields.at("prerequisites"),"question:libraryq")->fields.at("status")->string=="answered");
  CHECK(id_list(W->fields.at("shape")->fields.at("mapping"),"work")== (lib?std::vector<std::string>{"library"}:std::vector<std::string>{"command","library"}));
 }
 auto b2=spec_item(f.bundle,"dependencies","integrate","quality",string_value("accepted"));
 refusal("select_role",selected_bundle(selection_request(f),b2),"dependency_cycle");
}
TEST_CASE("review native records boundary roots and incomplete multi-file facets", "[reviewer]") {
 fixture f;auto r=selection_request(f);
 auto ws=f.bundle->fields.at("workspace"),v=ws->fields.at("value");
 auto b=changed(f.bundle,"workspace",changed(ws,"value",changed(v,"root",changed(v->fields.at("root"),"id",string_value("different")))));
 auto files=std::make_shared<value>(*r->fields.at("files"));for(auto& file:files->items)for(auto side:{"current:","base:"})if(file->fields.at("root")->string==std::string(side)+"source")file=changed(file,"root",string_value(std::string(side)+"different"));
 auto topologyfiles=std::make_shared<value>(*r->fields.at("capture")->fields.at("files"));for(auto& file:topologyfiles->items){auto root=file->fields.at("root")->string;root.replace(root.find(':')+1,std::string::npos,"different");file=changed(file,"root",string_value(root));}
 r=changed(selected_bundle(r,b),"files",files);r=changed(r,"capture",changed(r->fields.at("capture"),"files",topologyfiles));auto out=selected(r);
 auto tree=[&](std::string prefix) {std::vector<value_ptr> e;for(auto& file:files->items)if(file->fields.at("root")->string==prefix+"different")e.push_back(record_value({{"root",string_value("different")},{"path",file->fields.at("path")},{"kind",string_value("file")},{"mode",file->fields.at("mode")},{"size",integer_value(file->fields.at("bytes")->string.size())},{"content",invoke("file_digest",file->fields.at("bytes"))}}));std::ranges::sort(e,{},[](auto& x){return x->fields.at("path")->string;});return array_value(e);};
 auto base=tree("base:"),current=tree("current:");auto B=native_hash("base_v1",record_value({{"root",string_value("different")},{"label",string_value("initial")},{"entries",base},{"excluded",strings({"generated/"})}}));exact_value(out->fields.at("base")->fields.at("entries"),base);exact_value(out->fields.at("base")->fields.at("digest"),B);
 exact_value(out->fields.at("observation")->fields.at("digest"),native_hash("observation_v1",record_value({{"root",string_value("different")},{"base",B},{"entries",current},{"excluded",strings({"generated/"})}})));
 auto multi=spec_item(f.bundle,"facets","library_files","paths",strings({"src/core/quota.py","src/core/missing.py"}));multi=spec_item(multi,"dependencies","integrate","phase",string_value("start"));multi=spec_item(multi,"dependencies","integrate","quality",string_value("available"));multi=spec_item(multi,"work","command","checks",strings({"contract"}));
 auto missing=selected(selected_bundle(selection_request(f),multi));auto facet=missing->fields.at("facets")->items[0];CHECK(facet->fields.at("status")->string=="unavailable");CHECK_FALSE(facet->fields.contains("semantic"));CHECK_FALSE(facet->fields.contains("entries"));exact_value(missing->fields.at("work")->fields.at("value")->fields.at("start_facets"),array_value({record_value({{"dependency",string_value("integrate")},{"unavailable",[]{auto v=std::make_shared<value>();v->kind=value_kind::boolean;v->boolean=true;return v;}()}})}));
 CHECK(item_named(missing->fields.at("prerequisites"),"integrate")->fields.at("status")->string=="unavailable");
}
TEST_CASE("review inventory hidden extras malformed capture and mode materiality", "[reviewer]") {
 fixture f;auto r=selection_request(f);auto files=r->fields.at("files");
 refusal("select_role",changed(r,"files",appended(files,record_value({{"root",string_value("workspace")},{"path",string_value("extra.txt")},{"mode",string_value("0644")},{"bytes",string_value("")}}))),"reference");
 refusal("select_role",changed(r,"files",appended(files,record_value({{"root",string_value("observations")},{"path",string_value("other.json")},{"mode",string_value("0644")},{"bytes",string_value("")}}))),"path_denied");
 auto modechange=[&](std::string root){auto x=std::make_shared<value>(*files);for(auto& file:x->items)if(file->fields.at("root")->string==root)file=changed(file,"mode",string_value("0755"));return changed(r,"files",x);};
 CHECK(work_digest(modechange("workspace"))==work_digest(r));CHECK(work_digest(modechange("base:source"))!=work_digest(r));
 auto start=spec_item(f.bundle,"dependencies","integrate","phase",string_value("start"));start=spec_item(start,"dependencies","integrate","quality",string_value("done"));start=spec_item(start,"work","command","checks",strings({"contract"}));
 auto started=selected(selected_bundle(r,start));CHECK(item_named(started->fields.at("prerequisites"),"integrate")->fields.at("status")->string=="unverified");CHECK(started->fields.at("work")->fields.at("value")->fields.at("start_facets")->items.size()==1);
 CHECK(work_digest(selected_bundle(retained_change(r,"current:source","src/core/quota.py","changed upstream"),start))!=work_digest(selected_bundle(r,start)));
}

TEST_CASE("review three-stage pipeline scopes exact adjacent obligations and producer mapping", "[reviewer]") {
 fixture f;auto b=f.bundle;auto units=b->fields.at("spec")->fields.at("value")->fields.at("work");
 units=appended(units,record_value({{"id",string_value("third")},{"summary",md("Independent third work")},{"paths",strings({"third/"})},{"checks",strings({"contract"})}}));
 b=spec_field(b,"work",units);b=spec_item(b,"milestones","api","work",strings({"library","third"}));b=job_role(b,"combined","builder","work",strings({"command","library","third"}));
 for(auto job:{"combined","split"})b=job_role(b,job,"reviewer","targets",strings({"command","library","third"}));
 auto jobs=b->fields.at("jobs");value_ptr roles;for(auto&j:jobs->items)if(j->fields.at("value")->fields.at("id")->string=="split")roles=j->fields.at("value")->fields.at("roles");
 b=job_field(b,"split","roles",appended(roles,record_value({{"id",string_value("third_worker")},{"kind",string_value("worker")},{"work",strings({"third"})}})));
 b=job_field(b,"split","shape",string_value("pipeline"));b=job_field(b,"split","stages",array_value({strings({"library_worker"}),strings({"cli_worker"}),strings({"third_worker"})}));
 auto out=selected(selected_bundle(selection_request(f,"split","third_worker"),b));auto W=out->fields.at("work")->fields.at("value");
 auto predecessor=record_value({{"id",string_value("pipeline:command:third")},{"source",string_value("pipeline")},{"producer",string_value("command")},{"consumer",string_value("third")},{"producer_role",string_value("cli_worker")},{"consumer_role",string_value("third_worker")},{"phase",string_value("start")},{"quality",string_value("done")}});
 exact_value(W->fields.at("pipeline_predecessors"),array_value({predecessor}));
 exact_value(W->fields.at("shape")->fields.at("mapping"),array_value({record_value({{"work",string_value("command")},{"role",string_value("cli_worker")}}),record_value({{"work",string_value("third")},{"role",string_value("third_worker")}})}));
 exact_value(out->fields.at("prerequisites"),array_value({changed(predecessor,"status",string_value("unverified"))}));
 CHECK(W->fields.at("dependencies")->items.empty());CHECK(W->fields.at("facets")->items.empty());CHECK(W->fields.at("start_facets")->items.empty());CHECK(id_list(W->fields.at("checks"))==std::vector<std::string>{"contract"});
}
