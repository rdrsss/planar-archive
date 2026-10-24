import std;
import planar.kernel.capture;
using namespace planar::kernel;
int main() {
  auto base=std::filesystem::canonical(std::filesystem::temp_directory_path())/std::format("planar-review-{}",std::chrono::steady_clock::now().time_since_epoch().count());
  std::filesystem::create_directories(base);
  int checks=0;
  auto require=[&](bool ok,std::string_view label){ if(!ok){std::println("FAIL {}",label);std::exit(1);}++checks;};
  for(int kind=0;kind<6;++kind){
    auto p=base/std::to_string(kind);std::filesystem::create_directories(p/"keep/empty");std::filesystem::create_directories(p/"skip/empty");std::filesystem::create_directories(p/".git/objects");
    {std::ofstream f(p/"file");f<<"exact";}
    capture_session c;require(bool(c.add_root("r",p)),"root");require(bool(c.tree("r",{"skip/"})),"tree");
    c.before_verification([&]{
      if(kind==0){std::filesystem::remove_all(p/"skip");std::filesystem::create_symlink("missing",p/"skip");}
      if(kind==1){std::filesystem::remove_all(p/"skip");std::ofstream f(p/"skip");f<<"ignored";}
      if(kind==2){std::filesystem::remove_all(p/"keep");std::ofstream f(p/"keep");f<<"changed";}
      if(kind==3){std::filesystem::remove(p/"file");std::filesystem::create_directory(p/"file");}
      if(kind==4){std::filesystem::remove_all(p/".git");std::ofstream f(p/".git");f<<"changed";}
      if(kind==5){std::filesystem::remove_all(p/".git");std::filesystem::create_directories(p/".git/new");}
    });
    auto result=c.finish();bool succeeds=kind==0||kind==1||kind==5;
    require(bool(result)==succeeds,"seal classification");
    if(succeeds){auto topology=c.directories();require(bool(topology)&&(*topology)->size()==2,"retained topology");auto saved=*topology;std::filesystem::remove_all(p);require(c.directories()&&*c.directories()==saved,"getter has no I/O");}
    else{require(result.error().code=="capture_changed","changed code");require(!c.directories(),"no topology after failure");}
  }
  auto p=base/"individual";std::filesystem::create_directories(p/"nested");{std::ofstream f(p/"nested/file");f<<"read only";}
  capture_session single;require(bool(single.add_root("r",p)),"single root");require(bool(single.read("r","nested/file")),"single read");require(bool(single.finish()),"single finish");require(single.directories()&&(*single.directories())->empty(),"topology only enumerated trees");
  std::filesystem::remove_all(base);std::println("{} independent assertions passed in 7 adversarial scenarios",checks);
}
