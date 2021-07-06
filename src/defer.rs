/// Internal struct used by the `defer!` macro.
pub struct Defer<F: FnOnce()> {
	func: Option<F>,
}

impl<F: FnOnce()> Defer<F> {
	pub fn new(func: F) -> Self {
		Self { func: Some(func) }
	}
}

impl<F: FnOnce()> Drop for Defer<F> {
	fn drop(&mut self) {
		self.func.take().map(|f| f());
	}
}

/// Defers the execution of a block until the surrounding scope ends.
macro_rules! defer {
	( $($tt:tt)* ) => {
		let _deferred = crate::defer::Defer::new(|| { $($tt)* });
	};
}
