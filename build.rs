fn main() {
	println!("cargo:rustc-link-lib=dylib:+verbatim=res/resources.res");
}
