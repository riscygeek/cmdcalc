# Maintainer: Benjamin Stürz <stuerzbenni@gmail.com>
pkgname=cmdcalc
pkgver=2.1
pkgrel=1
pkgdesc="A simple command-line calculator"
arch=(x86_64 i686)
url="https://github.com/Benni3D/cmdcalc"
license=('GPL3')
depends=('readline>=8')
source=('https://github.com/Benni3D/cmdcalc/archive/v2.1.tar.gz')
md5sums=('08716498b365425c36aec8f8982722d5')

prepare() {
	install -dm 755 "${pkgname}-${pkgver}/build"
	cd "${pkgname}-${pkgver}/build"
	cmake .. -DCMAKE_BUILD_TYPE=Release
}

build() {
	cd "${pkgname}-${pkgver}/build"
	make -j$(nproc)
}

package() {
	install -dm 755 "${pkgdir}/usr/bin"
	install -Dm 755 "${srcdir}/${pkgname}-${pkgver}/build/cmdcalc2" "${pkgdir}/usr/bin/cmdcalc"
}
