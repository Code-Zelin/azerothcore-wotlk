-- AzerothCore 随机属性系统数据库表
-- 创建时间: 2024-12-19
-- 版本: 1.0

-- 1. 随机属性表
CREATE TABLE `item_random_attributes` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `item_guid` bigint unsigned NOT NULL,
  `attribute_type` tinyint unsigned NOT NULL,
  `attribute_value` int NOT NULL,
  `attribute_quality` tinyint unsigned NOT NULL DEFAULT 1,
  `random_multiplier` float NOT NULL DEFAULT 1.0,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `item_guid_attribute` (`item_guid`, `attribute_type`),
  KEY `idx_item_guid` (`item_guid`),
  KEY `idx_attribute_type` (`attribute_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Item Random Attributes System';

-- 2. 属性类型定义表
CREATE TABLE `item_attribute_types` (
  `id` tinyint unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL,
  `display_name` varchar(100) NOT NULL,
  `category` tinyint unsigned NOT NULL DEFAULT 1,
  `base_multiplier` float NOT NULL DEFAULT 1.0,
  `quality_multiplier` float NOT NULL DEFAULT 1.0,
  `min_quality` tinyint unsigned NOT NULL DEFAULT 2,
  `max_quality` tinyint unsigned NOT NULL DEFAULT 5,
  `weight` int unsigned NOT NULL DEFAULT 100,
  `enabled` tinyint unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `idx_category` (`category`),
  KEY `idx_quality_range` (`min_quality`, `max_quality`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Item Attribute Types Definition';

-- 3. 插入基础属性类型数据
-- 基础属性 (物品等级 × 0.33 × 0.5-1.5)
INSERT INTO `item_attribute_types` VALUES
(1, 'STRENGTH', '力量', 1, 0.33, 1.0, 2, 5, 100, 1),
(2, 'AGILITY', '敏捷', 1, 0.33, 1.0, 2, 5, 100, 1),
(3, 'STAMINA', '耐力', 1, 0.33, 1.0, 2, 5, 100, 1),
(4, 'SPIRIT', '精神', 1, 0.33, 1.0, 2, 5, 100, 1),
(5, 'INTELLECT', '智力', 1, 0.33, 1.0, 2, 5, 100, 1);

-- 伤害属性 (物品等级 × 0.5 × 0.5-1.5)
INSERT INTO `item_attribute_types` VALUES
(6, 'FIRE_DAMAGE', '火焰伤害', 2, 0.5, 1.0, 2, 5, 80, 1),
(7, 'SHADOW_DAMAGE', '暗影伤害', 2, 0.5, 1.0, 2, 5, 80, 1),
(8, 'HOLY_DAMAGE', '神圣伤害', 2, 0.5, 1.0, 2, 5, 80, 1),
(9, 'FROST_DAMAGE', '冰霜伤害', 2, 0.5, 1.0, 2, 5, 80, 1),
(10, 'NATURE_DAMAGE', '自然伤害', 2, 0.5, 1.0, 2, 5, 80, 1),
(11, 'ARCANE_DAMAGE', '奥术伤害', 2, 0.5, 1.0, 2, 5, 80, 1);

-- 防御属性 (物品等级 × 0.3 × 0.5-1.5)
INSERT INTO `item_attribute_types` VALUES
(12, 'DEFENSE_RATING', '防御等级', 3, 0.3, 1.0, 2, 5, 60, 1),
(13, 'DODGE_RATING', '闪避等级', 3, 0.3, 1.0, 2, 5, 60, 1),
(14, 'BLOCK_RATING', '格挡等级', 3, 0.3, 1.0, 2, 5, 60, 1),
(15, 'PARRY_RATING', '招架等级', 3, 0.3, 1.0, 2, 5, 60, 1),
(16, 'ARMOR', '护甲值', 3, 2.0, 1.0, 2, 5, 60, 1);

-- 攻击属性 (物品等级 × 0.3 × 0.5-1.5)
INSERT INTO `item_attribute_types` VALUES
(17, 'HIT_RATING', '命中等级', 4, 0.3, 1.0, 2, 5, 70, 1),
(18, 'CRIT_RATING', '暴击等级', 4, 0.3, 1.0, 2, 5, 70, 1),
(19, 'HASTE_RATING', '急速等级', 4, 0.3, 1.0, 2, 5, 70, 1),
(20, 'EXPERTISE_RATING', '精准等级', 4, 0.3, 1.0, 2, 5, 70, 1),
(21, 'PENETRATION_RATING', '穿透等级', 4, 0.3, 1.0, 2, 5, 70, 1);

-- 法术属性 (物品等级 × 0.3 × 0.5-1.5)
INSERT INTO `item_attribute_types` VALUES
(22, 'SPELL_HIT', '法术命中', 5, 0.3, 1.0, 2, 5, 50, 1),
(23, 'SPELL_CRIT', '法术暴击', 5, 0.3, 1.0, 2, 5, 50, 1),
(24, 'SPELL_HASTE', '法术急速', 5, 0.3, 1.0, 2, 5, 50, 1),
(25, 'SPELL_PENETRATION', '法术穿透', 5, 0.3, 1.0, 2, 5, 50, 1),
(26, 'SPELL_POWER', '法术强度', 5, 0.3, 1.0, 2, 5, 50, 1);

-- 恢复属性 (物品等级 × 0.33 × 0.5-1.5)
INSERT INTO `item_attribute_types` VALUES
(27, 'HEALTH_REGEN', '生命回复', 6, 0.33, 1.0, 2, 5, 30, 1);

-- 特殊属性 (移动速度固定1-3%)
INSERT INTO `item_attribute_types` VALUES
(28, 'MOVEMENT_SPEED', '移动速度', 7, 0.0, 1.0, 2, 5, 20, 1); 