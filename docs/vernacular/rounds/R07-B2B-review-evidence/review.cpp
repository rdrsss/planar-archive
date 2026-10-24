// Independent finite production API review probes.
#include "<repo>/src/kernel/runtime/schema.t.cpp"

TEST_CASE("independent exhaustive two-unit phase graphs agree with reference topology", "[independent]") {
 fixture f;
 const std::vector<std::pair<std::string,std::string>> options={{"none","none"},{"start","available"},{"start","done"},{"start","accepted"},{"complete","available"},{"complete","done"},{"complete","accepted"}};
 for (int shape=0; shape<2; ++shape) for (auto [lp,lq]:options) for (auto [cp,cq]:options) {
  INFO(shape << ":" << lp << "/" << lq << ":" << cp << "/" << cq);
  auto b=f.bundle;
  auto facet=record_value({{"id",string_value("command_files")},{"producer",string_value("command")},{"paths",strings({"src/cli/main.py"})}});
  auto facets=std::make_shared<value>(*b->fields.at("spec")->fields.at("value")->fields.at("facets")); facets->items.push_back(facet);
  b=spec_field(b,"facets",facets);
  std::vector<value_ptr> ds={dependency("develop","command","contract","quota_api","start","available")};
  if(lp!="none") ds.push_back(dependency("left","library","facet","command_files",lp,lq));
  if(cp!="none") ds.push_back(dependency("right","command","facet","library_files",cp,cq));
  b=spec_field(b,"dependencies",array_value(ds));
  b=spec_item(b,"work","library","checks",lp=="complete"?strings({"contract","integration"}):strings({"contract"}));
  b=spec_item(b,"work","command","checks",cp=="complete"?strings({"contract","integration"}):strings({"contract"}));
  if(shape) { b=job_field(b,"split","shape",string_value("pipeline")); b=job_field(b,"split","stages",array_value({strings({"library_worker"}),strings({"cli_worker"})})); }
  bool cyclic=false;
  for (bool grouped: {true,false}) {
   std::map<std::string,std::set<std::string>> adj;
   auto edge=[&](std::string a,std::string z) {adj[a].insert(z);adj[z];};
   for(auto u:{"library","command"}) {
    std::string w="work:"+std::string(u)+":";
    std::string role=grouped?"builder":(std::string(u)=="library"?"library_worker":"cli_worker");
    edge(w+"start",w+"complete");edge(w+"complete",w+"accepted");edge(w+"complete","role:"+role+":complete");
    edge("role:"+role+":complete","role:reviewer:start");edge("role:reviewer:complete",w+"accepted");
   }
   edge("role:reviewer:start","role:reviewer:complete");
   auto explicit_edge=[&](std::string p,std::string q,std::string producer,std::string consumer) {
    if(p!="none" && q!="available") edge("work:"+producer+":"+(q=="done"?"complete":"accepted"),"work:"+consumer+":"+p);
   };
   explicit_edge(lp,lq,"command","library");explicit_edge(cp,cq,"library","command");
   if(shape && !grouped) edge("work:library:complete","work:command:start");
   auto original=adj;
   std::map<std::string,int> indeg; for(auto [v,tos]:adj) {indeg[v];for(auto z:tos) ++indeg[z];}
   std::vector<std::string> todo;for(auto [v,n]:indeg) if(!n)todo.push_back(v);
   size_t n=0;while(!todo.empty()) {auto v=todo.back();todo.pop_back();++n;for(auto z:adj[v])if(!--indeg[z])todo.push_back(z);}
   bool local_cycle=n!=adj.size();cyclic=cyclic||local_cycle;
   if(!cyclic && !local_cycle) {
    auto raw=invoke("validate_bundle",validation(b));
    if(raw->fields.contains("value")) {
     auto g=item_named(raw->fields.at("value")->fields.at("jobs"),grouped?"combined":"split")->fields.at("graph");
     size_t count=0;for(auto [v,tos]:original)for(auto z:tos){CHECK(has_edge(g,v,z));++count;}
     CHECK(g->fields.at("edges")->items.size()==count);CHECK(g->fields.at("vertices")->items.size()==original.size());
    }
   }
  }
  auto raw=invoke("validate_bundle",validation(b));
  if(cyclic) { REQUIRE(raw->fields.contains("error")); CHECK(raw->fields.at("error")->fields.at("code")->string=="dependency_cycle"); }
  else { REQUIRE(raw->fields.contains("value")); }
 }
}
TEST_CASE("independent topology envelope and missing declarations refuse through production", "[independent]") {
 fixture f;
 refusal("validate_bundle",validation(f.bundle,record_value({{"status",string_value("not_captured")},{"files",array_value()}})),"schema");
 refusal("validate_bundle",validation(f.bundle,captured(array_value({topology("current:source","unused"),topology("current:source","unused")}),array_value())),"reference");
 refusal("validate_bundle",validation(f.bundle,captured(array_value({topology("current:source","unused")}),array_value({topology("current:source","unused")}))),"reference");
 refusal("validate_bundle",validation(f.bundle,captured(array_value({topology("base:source","SRC/unowned")}),array_value())),"ownership");
 auto b=changed(f.bundle,"jobs",array_value());refusal("validate_bundle",validation(b),"reference");
}
