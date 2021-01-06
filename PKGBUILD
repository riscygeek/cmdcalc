# Maintainer: Benjamin Stürz <stuerzbenni@gmail.com>
pkgname=cmdcalc
pkgver=2.0
pkgrel=1
pkgdesc="A simple command-line calculator"
arch=(x86_64 i686)
url="https://github.com/Benni3D/cmdcalc"
license=('GPL3')
depends=('readline>=8')
source=('https://github.com/Benni3D/cmdcalc/archive/v2.0.tar.gz')
md5sums=('12a2a89325671e5b10426a94455bf9d4')

prepare() {
	install -dm 755 "${pkgname}-${pkgver}/build"
	cd "${pkgname}-${pkgver}/build"
	cmake .. -DCMAKE_BUILD_TYPE=Release
}

build() {
	cd "${pkgname}-${pkgver}/build"
	make
}

package() {
	install -dm 755 "${pkgdir}/usr/bin"
	install -Dm 644 "${srcdir}/${pkgname}-${pkgver}/build/cmdcalc2" "${pkgdir}/usr/bin/cmdcalc"
}
