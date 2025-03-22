include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk

PKG_NAME:=icmp-knock
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

define Package/icmp-knock
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Utility for testing intermediate nodes
	DEPENDS:=+libpcap
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp $(TOPDIR)/package/icmp-knock/icmp-knock.c $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/icmp-knock $(PKG_BUILD_DIR)/icmp-knock.c -lpcap
endef

define Package/icmp-knock/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/icmp-knock $(1)/bin
endef

$(eval $(call BuildPackage,icmp-knock))
