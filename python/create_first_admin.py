#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
AzerothCore第一个管理员账号创建工具
直接操作数据库创建账号（使用SRP6认证）
"""

import hashlib
import secrets
import mysql.connector
import sys

def calculate_srp6_values(username, password):
    """
    计算SRP6认证所需的salt和verifier值
    
    Args:
        username: 用户名（大写）
        password: 密码（大写）
        
    Returns:
        tuple: (salt, verifier)
    """
    # WoW使用的参数
    N = int('894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7', 16)
    g = 7
    
    # 计算身份哈希 H(username:password)
    identity = f"{username.upper()}:{password.upper()}"
    identity_hash = hashlib.sha1(identity.encode()).digest()
    
    # 生成随机salt
    salt = secrets.token_bytes(32)
    
    # 计算x = H(salt, identity_hash)
    x_hash = hashlib.sha1(salt + identity_hash).digest()
    x = int.from_bytes(x_hash, 'little') % N
    
    # 计算verifier = g^x mod N
    verifier = pow(g, x, N)
    verifier_bytes = verifier.to_bytes(32, 'little')
    
    return salt, verifier_bytes

def create_admin_account(host='127.0.0.1', port=3306, user='root', password='password', 
                        admin_username='admin', admin_password='admin'):
    """
    在数据库中创建管理员账号
    
    Args:
        host: 数据库主机
        port: 数据库端口
        user: 数据库用户名
        password: 数据库密码
        admin_username: 要创建的管理员用户名
        admin_password: 要创建的管理员密码
    """
    try:
        # 连接数据库
        print(f"连接到数据库 {host}:{port}...")
        conn = mysql.connector.connect(
            host=host,
            port=port,
            user=user,
            password=password,
            database='acore_auth'
        )
        cursor = conn.cursor()
        
        # 检查账号是否已存在
        cursor.execute("SELECT id FROM account WHERE username = %s", (admin_username,))
        if cursor.fetchone():
            print(f"❌ 账号 '{admin_username}' 已存在")
            return False
        
        # 计算SRP6值
        print("计算SRP6认证值...")
        salt, verifier = calculate_srp6_values(admin_username, admin_password)
        
        # 插入账号
        print(f"创建账号 '{admin_username}'...")
        insert_account = """
        INSERT INTO account (username, salt, verifier, expansion) 
        VALUES (%s, %s, %s, 2)
        """
        cursor.execute(insert_account, (admin_username, salt, verifier))
        account_id = cursor.lastrowid
        
        # 设置GM权限
        print("设置GM权限...")
        insert_access = """
        INSERT INTO account_access (id, gmlevel, RealmID) 
        VALUES (%s, 3, -1)
        """
        cursor.execute(insert_access, (account_id,))
        
        # 提交事务
        conn.commit()
        
        print(f"✅ 管理员账号创建成功！")
        print(f"用户名: {admin_username}")
        print(f"密码: {admin_password}")
        print(f"GM等级: 3 (最高权限)")
        
        return True
        
    except mysql.connector.Error as e:
        print(f"❌ 数据库错误: {e}")
        return False
    except Exception as e:
        print(f"❌ 未知错误: {e}")
        return False
    finally:
        if 'conn' in locals() and conn.is_connected():
            cursor.close()
            conn.close()
            print("数据库连接已关闭")

def main():
    """主函数"""
    print("=== AzerothCore 第一个管理员账号创建工具 ===\n")
    
    # 获取用户输入
    admin_username = input("管理员用户名 [admin]: ").strip() or "admin"
    admin_password = input("管理员密码 [admin]: ").strip() or "admin"
    
    # 数据库连接信息
    print("\n数据库连接信息:")
    db_host = input("数据库主机 [127.0.0.1]: ").strip() or "127.0.0.1"
    db_port = input("数据库端口 [3306]: ").strip() or "3306"
    db_user = input("数据库用户名 [root]: ").strip() or "root"
    db_pass = input("数据库密码 [password]: ").strip() or "password"
    
    try:
        db_port = int(db_port)
    except ValueError:
        print("❌ 端口必须是数字")
        return
    
    # 创建账号
    success = create_admin_account(
        host=db_host,
        port=db_port,
        user=db_user,
        password=db_pass,
        admin_username=admin_username,
        admin_password=admin_password
    )
    
    if success:
        print(f"\n🎉 现在你可以使用以下信息:")
        print(f"1. 登录游戏客户端")
        print(f"2. 使用SOAP工具创建其他账号")
        print(f"3. 在游戏中使用GM命令")
    else:
        print(f"\n故障排除建议:")
        print(f"1. 确保数据库服务正在运行")
        print(f"2. 检查数据库连接信息是否正确")
        print(f"3. 确保数据库用户有足够权限")

if __name__ == "__main__":
    main() 