# Release Notes - Version 1.5.0 (QA)

**Release Date:** March 5, 2026

---

## Summary

This release introduces AES-256-GCM password encryption, PBKDF2 password hashing, and new AI analytics modules. This QA guide covers **Login** and **User Management (New User Flow)** testing.

---

## QA Testing Guide

### 1. Login Tests

#### 1.1 Normal Login
1. Open `https://<camera-ip>` in browser
2. Enter valid admin credentials
3. Verify login succeeds and dashboard loads
4. Check browser console — no encryption errors

#### 1.2 Wrong Password
1. Enter correct username with incorrect password
2. Verify HTTP 401 response with `"Invalid credentials"` message
3. Repeat 3 times — verify account is not locked (no lockout policy yet)

#### 1.3 Wrong Username
1. Enter non-existent username with any password
2. Verify HTTP 401 response
3. Verify error message does not reveal whether user exists

#### 1.4 Empty Fields
1. Submit login with empty username — verify error
2. Submit login with empty password — verify error
3. Submit login with both empty — verify error

#### 1.5 Session Verification
1. Login successfully
2. Call `GET /api/v1/auth/verify` — verify `{"valid": true}`
3. Copy the Bearer token
4. Logout via `POST /api/v1/auth/logout`
5. Use the old token to call `GET /api/v1/auth/verify` — verify `{"valid": false}` or 401

#### 1.6 AES-256-GCM Encryption Verification
1. Call `GET /api/v1/auth/public-key`
2. Verify response contains `"aes_algorithm": "AES-256-GCM"`
3. Login via API sending `EncryptedPassword` with `auth_tag` field
4. Verify login succeeds
5. **Test CBC fallback (deprecated):** Login without `auth_tag` — verify login still works (backward compat)

#### 1.7 Concurrent Sessions
1. Login from Browser A
2. Login from Browser B with same credentials
3. Verify both sessions work independently
4. Logout from Browser A — verify Browser B session is unaffected

#### 1.8 Token Expiry
1. Login and note the token
2. Wait for token to expire (or manually expire via API if available)
3. Call any authenticated endpoint — verify HTTP 401
4. Re-login — verify new token works

---

### 2. User Management — New User Flow

#### 2.1 Create New User (Admin)
1. Login as admin
2. Call `POST /api/v1/users/create` with:
   ```json
   {
     "username": "testuser1",
     "password": {
       "encrypted": "<AES-256-GCM encrypted password>",
       "iv": "<12-byte base64 IV>",
       "encrypted_key": "<RSA-OAEP encrypted AES key>",
       "auth_tag": "<16-byte base64 GCM tag>"
     },
     "role": "viewer"
   }
   ```
3. Verify HTTP 201 with `user_id` (integer) in response
4. Verify `password_must_change` is `true` in response

#### 2.2 New User First Login
1. Logout admin session
2. Login as `testuser1` with the password set during creation
3. Verify login succeeds
4. Verify response includes `"password_must_change": true` or `"initial_setup_required": true`

#### 2.3 New User Must Change Password
1. After first login as `testuser1`
2. Call `POST /api/v1/auth/change-password` with old and new password (GCM encrypted)
3. Verify password change succeeds
4. Logout and re-login with new password — verify success
5. Verify `password_must_change` is now `false`

#### 2.4 Create User — Duplicate Username
1. Login as admin
2. Call `POST /api/v1/users/create` with `"username": "testuser1"` (already exists)
3. Verify HTTP 409 or 400 with appropriate error message

#### 2.5 Create User — Invalid Role
1. Call `POST /api/v1/users/create` with `"role": "superadmin"` (invalid)
2. Verify HTTP 400 with validation error

#### 2.6 Create User — Missing Fields
1. Call `POST /api/v1/users/create` without `username` — verify error
2. Call without `password` — verify error
3. Call without `role` — verify uses default role or returns error

#### 2.7 List Users
1. Login as admin
2. Call `GET /api/v1/users`
3. Verify `testuser1` appears in the list
4. Verify password hash is NOT exposed in response
5. Verify response includes `user_id`, `username`, `role`, `enabled`, `last_login`

#### 2.8 Get Single User
1. Call `GET /api/v1/users/{id}` with the `user_id` from step 2.1
2. Verify correct user details returned
3. Call with non-existent ID — verify HTTP 404

#### 2.9 Update User
1. Login as admin
2. Call `PUT /api/v1/users/{id}` to change role from `viewer` to `operator`
3. Verify HTTP 200
4. Call `GET /api/v1/users/{id}` — verify role is updated
5. Verify password field is NOT accepted in PUT (must use change-password endpoint)

#### 2.10 Disable User
1. Call `PUT /api/v1/users/{id}` with `{"enabled": false}`
2. Logout admin, attempt login as `testuser1` — verify login fails (account disabled)
3. Re-enable: `PUT /api/v1/users/{id}` with `{"enabled": true}`
4. Login as `testuser1` — verify login succeeds

#### 2.11 Delete User
1. Login as admin
2. Call `DELETE /api/v1/users/{id}` for `testuser1`
3. Verify HTTP 200
4. Call `GET /api/v1/users/{id}` — verify HTTP 404
5. Attempt login as `testuser1` — verify login fails

#### 2.12 Self-Deletion Prevention
1. Login as admin
2. Get admin's own `user_id` from `GET /api/v1/users/current`
3. Call `DELETE /api/v1/users/{admin_id}`
4. Verify HTTP 403 or 400 — admin cannot delete own account

#### 2.13 Role-Based Access Control
1. Create a `viewer` role user
2. Login as viewer
3. Call `POST /api/v1/users/create` — verify HTTP 403 (insufficient permissions)
4. Call `DELETE /api/v1/users/{id}` — verify HTTP 403
5. Call `GET /api/v1/users` — verify behavior (may be restricted to own user or 403)

---

## Test Results Template

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| 1.1 | Normal Login | ⬜ | |
| 1.2 | Wrong Password | ⬜ | |
| 1.3 | Wrong Username | ⬜ | |
| 1.4 | Empty Fields | ⬜ | |
| 1.5 | Session Verification | ⬜ | |
| 1.6 | AES-256-GCM Verification | ⬜ | |
| 1.7 | Concurrent Sessions | ⬜ | |
| 1.8 | Token Expiry | ⬜ | |
| 2.1 | Create New User | ⬜ | |
| 2.2 | New User First Login | ⬜ | |
| 2.3 | Must Change Password | ⬜ | |
| 2.4 | Duplicate Username | ⬜ | |
| 2.5 | Invalid Role | ⬜ | |
| 2.6 | Missing Fields | ⬜ | |
| 2.7 | List Users | ⬜ | |
| 2.8 | Get Single User | ⬜ | |
| 2.9 | Update User | ⬜ | |
| 2.10 | Disable User | ⬜ | |
| 2.11 | Delete User | ⬜ | |
| 2.12 | Self-Deletion Prevention | ⬜ | |
| 2.13 | Role-Based Access Control | ⬜ | |

**QA Tester:** _______________  
**Date Tested:** _______________  
**Firmware Version:** v1.5.0  
**Device IP:** _______________
