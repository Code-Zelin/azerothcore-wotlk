#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
AzerothCore账号注册工具
通过SOAP接口创建游戏账号
"""

import requests
import base64
import sys
from xml.etree import ElementTree as ET

class AzerothCoreAccountManager:
    def __init__(self, soap_url="http://127.0.0.1:7878", admin_user="admin", admin_pass="admin"):
        """
        初始化AzerothCore账号管理器
        
        Args:
            soap_url: SOAP服务地址
            admin_user: 管理员账号（需要GM权限）
            admin_pass: 管理员密码
        """
        self.soap_url = soap_url
        self.admin_user = admin_user
        self.admin_pass = admin_pass
        
    def create_account(self, username, password):
        """
        创建新账号
        
        Args:
            username: 用户名
            password: 密码
            
        Returns:
            tuple: (success: bool, message: str)
        """
        # 构建SOAP请求
        soap_body = f'''<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope 
    xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" 
    xmlns:ns1="urn:AC">
    <SOAP-ENV:Body>
        <ns1:executeCommand>
            <command>.account create {username} {password}</command>
        </ns1:executeCommand>
    </SOAP-ENV:Body>
</SOAP-ENV:Envelope>'''

        # 设置HTTP头
        headers = {
            'Content-Type': 'text/xml; charset=utf-8',
            'SOAPAction': 'urn:AC#executeCommand',
            'Authorization': f'Basic {base64.b64encode(f"{self.admin_user}:{self.admin_pass}".encode()).decode()}'
        }
        
        try:
            # 发送SOAP请求
            response = requests.post(self.soap_url, data=soap_body, headers=headers, timeout=10)
            
            if response.status_code == 200:
                try:
                    # 解析响应
                    root = ET.fromstring(response.text)
                    
                    # 查找结果 - 正确的路径（result元素没有命名空间）
                    result_element = root.find('.//{urn:AC}executeCommandResponse/result')
                    if result_element is None:
                        # 备用方法：搜索所有result元素
                        for elem in root.iter():
                            if elem.tag == 'result':
                                result_element = elem
                                break
                    
                    if result_element is not None:
                        result_text = result_element.text or ""
                        
                        if "Account created" in result_text or "创建帐号" in result_text or "创建成功" in result_text:
                            return True, f"账号 '{username}' 创建成功！"
                        elif "already exist" in result_text or "已存在" in result_text or "已经存在" in result_text:
                            return False, f"账号 '{username}' 已存在"
                        else:
                            return False, f"创建失败: {result_text}"
                    else:
                        # 如果找不到结果元素，显示完整响应用于调试
                        return False, f"无法解析服务器响应。响应内容: {response.text[:200]}..."
                except ET.ParseError as e:
                    return False, f"XML解析错误: {e}. 响应内容: {response.text[:200]}..."
                    
            elif response.status_code == 401:
                return False, "认证失败：管理员账号或密码错误"
            elif response.status_code == 404:
                return False, "SOAP服务未启用或地址错误"
            else:
                return False, f"HTTP错误: {response.status_code}"
                
        except requests.exceptions.ConnectionError:
            return False, "无法连接到SOAP服务，请检查服务器是否运行"
        except requests.exceptions.Timeout:
            return False, "请求超时"
        except Exception as e:
            return False, f"未知错误: {str(e)}"

def main():
    """主函数"""
    print("=== AzerothCore 账号注册工具 ===\n")
    
    # 获取用户输入
    username = input("请输入用户名: ").strip()
    if not username:
        print("错误：用户名不能为空")
        return
        
    password = input("请输入密码: ").strip()
    if not password:
        print("错误：密码不能为空")
        return
    
    # 管理员账号配置（需要根据实际情况修改）
    admin_user = input("管理员账号 [admin]: ").strip() or "admin"
    admin_pass = input("管理员密码 [admin]: ").strip() or "admin"
    
    # 创建账号管理器
    manager = AzerothCoreAccountManager(admin_user=admin_user, admin_pass=admin_pass)
    
    print(f"\n正在创建账号 '{username}'...")
    
    # 创建账号
    success, message = manager.create_account(username, password)
    
    if success:
        print(f"✅ {message}")
        print(f"\n现在你可以使用以下信息登录游戏：")
        print(f"用户名: {username}")
        print(f"密码: {password}")
    else:
        print(f"❌ {message}")
        
        # 提供故障排除建议
        print("\n故障排除建议：")
        print("1. 确保worldserver正在运行")
        print("2. 确保SOAP服务已启用 (SOAP.Enabled = 1)")
        print("3. 确保管理员账号具有GM权限")
        print("4. 检查防火墙是否阻止7878端口")

if __name__ == "__main__":
    main() 