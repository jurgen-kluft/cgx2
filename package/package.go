package cgx2

import (
	denv "github.com/jurgen-kluft/ccode/denv"
	ccore "github.com/jurgen-kluft/ccore/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

const (
	repo_path = "github.com\\jurgen-kluft"
	repo_name = "cgx2"
)

func GetPackage() *denv.Package {
	name := repo_name

	// dependencies
	ccorepkg := ccore.GetPackage()
	cunittestpkg := cunittest.GetPackage()

	// main package
	mainpkg := denv.NewPackage(repo_path, repo_name)
	mainpkg.AddPackage(ccorepkg)
	mainpkg.AddPackage(cunittestpkg)

	// main library
	mainlib := denv.SetupCppLibProject(mainpkg, name)
	mainlib.AddDependencies(ccorepkg.GetMainLib())

	// test library
	testlib := denv.SetupCppTestLibProject(mainpkg, name)
	testlib.AddDependencies(ccorepkg.GetTestLib())

	// unittest project
	maintest := denv.SetupCppTestProject(mainpkg, name)
	maintest.AddDependencies(cunittestpkg.GetTestLib())
	maintest.AddDependency(testlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddTestLib(testlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
