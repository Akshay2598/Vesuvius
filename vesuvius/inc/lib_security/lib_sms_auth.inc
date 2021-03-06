<?
/**
 * @name         SMS Authentication Library
 * @version      1.0
 * @package      framework
 * @author       Ravindra De Silva <ravindra@opensource.lk><ravidesilva@iee.org>
 * @author       Greg Miernicki <g@miernicki.com> <gregory.miernicki@nih.gov>
 * @about        Developed in whole or part by the U.S. National Library of Medicine
 * @link         https://pl.nlm.nih.gov/about
 * @link         http://sahanafoundation.org
 * @license	 http://www.gnu.org/licenses/lgpl-2.1.html GNU Lesser General Public License (LGPL)
 * @lastModified 2011.0308
 */

require_once "constants.inc";

function _shn_auth_user_list_and_status() {
	global $global;
	$db=$global['db'];
	$q="select users.p_uuid,full_name,user_name,status from person_uuid,users where users.p_uuid=person_uuid.p_uuid and users.p_uuid <> 1  order by full_name";
	$res=$db->Execute($q);
	$users=array();
	while(!$res->EOF){
		//$name=$res->fields[2].".".$res->fields["full_name"];
		$name=$res->fields["full_name"].":".$res->fields["user_name"];
		$users[$res->fields["p_uuid"]]=array($name,$res->fields["status"]);
		$res->MoveNext();
	}
	return $users;
}



function _shn_auth_locked_user_list() {
	global $global;
	$db=$global['db'];
	$q="select users.p_uuid,full_name,user_name,status from person_uuid,users where status='locked' and users.p_uuid=person_uuid.p_uuid and users.p_uuid <> 1  order by full_name";
	$res=$db->Execute($q);
	$users=array();
	while(!$res->EOF){
		//$name=$res->fields[2].".".$res->fields["full_name"];
		$name=$res->fields["full_name"].":".$res->fields["user_name"];
		$users[$res->fields["p_uuid"]]=array($name,$res->fields["status"]);
		$res->MoveNext();
	}
	return $users;

}

function _shn_auth_activate_user($pid=null,$uname=null) {
	if($pid==null){
		if($uname==null){
			return false;
		}else{
			$sql="update users set status='active'  where user_name='{$uname}'";
		}
	}else{
		$sql="update users set status='active' where p_uuid='{$pid}'";
	}
	//echo $sql;
	global $global;
	$db=$global['db'];
	$res=$db->Execute($sql);
	return $res;

}



/**
 *check the existence of an user ----> Change it to check for Logged in user via Call ID Session
 *@return bool
 *@param string user name
 *@access public
 */
function _shn_current_user(){
	global $global;
	$q = "select p_uuid from  users where  user_name = '{$_SESSION['user']}'";
	$db=$global['db'];
	$res=$db->Execute($q);
	if(($res==null) or ($res->EOF)){
		return null;
	}else {
		return $res->fields["p_uuid"];
	}
}



function _shn_get_user_details($user){
	$q = "select full_name from person_uuid where  p_uuid = '{$user}'";
	global $global;
	$db=$global['db'];
	$res=$db->Execute($q);
	if(($res==null) or ($res->EOF)){
		return false;
	}else {
		return $res->fields["full_name"];
	}
}



/**
 *check the existence of an user
 *@return bool
 *@param string user name
 *@access public
 */
function _shn_is_user($user_name){
	$q = "select p_uuid from  users where  user_name = '{$user_name}'";
	global $global;
	$db=$global['db'];
	$res=$db->Execute($q);
	if(($res==null) or ($res->EOF)){
		return false;
	}else {
		return true;
	}
}



function _shn_auth_user_list($fullname=false,$admin=true){
	global $global;
	$db=$global['db'];
	$q="select users.p_uuid,full_name,user_name from person_uuid,users where users.p_uuid=person_uuid.p_uuid order by user_name";
	//$q="select users.p_uuid,full_name,user_name from person_uuid,users where users.p_uuid=person_uuid.p_uuid and users.p_uuid <> 1  order by user_name";
	$res=$db->Execute($q);
	$users=array();
	while(!$res->EOF){
		//$name=$res->fields[2].".".$res->fields["full_name"];
		if($res->fields["full_name"]==null){
			$name="Full Name not avaliable - ".$name=$res->fields["user_name"];
		}else{
			$name=$res->fields["full_name"]." - ".$name=$res->fields["user_name"];
		}
		//$name=$res->fields["full_name"];
		if(($res->fields["p_uuid"]!="1") or($admin==true)){
			$users[$res->fields["p_uuid"]]=$name;
		}
		$res->MoveNext();
	}

    return $users;
}



/**
 *Check if a user has an account that matches the user name and password
 *therefore this is the function you need to call for authentication
 *since authentication is called by the front controller
 *and all the GET variables avaliable to the front controller via SMS
 *are avaliable to this function as well. Keyword login followed by user name and password
 *are sent as GET  authentication is done only as the user sent the keyword loginarray to the Sahana front controller which passes it on to
  *mod/msg/lib_sms_process.inc
 *which performs the login action by calling this function.
 *remember this function will be called with every request to the front
 *controller. But we need to authenticate only when its a login attempt
 *if its not a login request return -1
 *Need to check session
 *@return int the user id , if the user exists ,else 0 or -1
 *@access public
 */

function _shn_authenticate_user($parsed_message) {
	/*need to modify the function to work with the sahana database scheme
	 and adodb code, till then return true.
	 */

	global $global;
	$db=$global['db'];
	$user_data=array("user_id"=>ANONYMOUS_USER,"user"=>"Anonymous");
	if("logout"==$parsed_message[0]){
		$user_data["user_id"]=ANONYMOUS_USER;
		$user_data["user"]="Anonymous";
		$user_data["result"]=LOGGEDOUT;
		return $user_data ;
	}

	/* if user has not requested login. Check Caller ID to authenticate
	 return -1 , so the calling application can identify that
	 authentication was not attempted
	 */
	if(("login"!=$parsed_message[0]) && ("100"!=$parsed_message[0])){
		$user_data["user_id"]=-1;
		return $user_data ;
	} else {
		//authentication is done only as the user sent the keyword login
		$user= addslashes(strip_tags(trim($parsed_message[1])));
        //this might malfunction in case the password of the user contains a space. @ todo
        // solution is restruction the parsed_message array including spaces
     	$pwd= addslashes(strip_tags(trim($parsed_message[2])));
		$q = "SELECT salt,p_uuid,status  FROM users
                    WHERE user_name = '$user'";
		$res=$db->Execute($q);
		if(($res==null)||($res->EOF)){
			//echo "Login Failed : Invalid user name or password.";
			//shn_acl_log_msg("Login Failed : Invalid user name or password.","aonymous","Aonymous User");
			$user_data["user_id"]=ANONYMOUS_USER;
			$user_data["user"]="Anonymous";
            $user_data["error_code"]="3";
			return $user_data;
		}else{
			$status=$res->fields["status"];
			$salt=$res->fields["salt"];
			$uid=$res->fields["p_uuid"];
		}
		if($status=='pending'){
			$user_data["user_id"]=ANONYMOUS_USER;
			$user_data["user"]="Anonymous";
            $user_data["error_code"]="7";//Login Failed : Account is not active yet. Contact Administrator to activate your account
			return $user_data;
		}


		if($status=='locked'){
			$sql="SELECT changed_timestamp FROM password_event_log WHERE p_uuid='{$uid}' and event_type=1 order by changed_timestamp desc ";
			$res=$db->Execute($sql);
			if(($res==null)||($res->EOF)){
				//$error_code = 6;//echo "Login Failed : Password log attacked.";
				//shn_acl_log_msg("Login Failed : Password log attacked.",$uid,$user,1);
				$user_data["user_id"]=ANONYMOUS_USER;
				$user_data["user"]="Anonymous";
                $user_data["error_code"]="6";
				return $user_data;
			}else{
				$tstamp=$res->fields["changed_timestamp"];
				$now=time();
				$diff=$now-$tstamp;
				if($diff < LOCK){
					//add_error("Login Failed : Password lock still valid.");
					//$error_code = 5; echo "This account has been locked.  Please contact the Administrator to unlock the account.";
					//shn_acl_log_msg("Login Failed : Password lock still valid.",$uid,$user);
					$user_data["user_id"]=ANONYMOUS_USER;
					$user_data["user"]="Anonymous";
                    $user_data["error_code"]="5";
					return $user_data;
				}

			}

		}

		// banned user
		if($status == 'banned'){
			//$error_code = 4;//echo "Login Failed : You have been banned by an administrator of the system.";
			//shn_acl_log_msg("Login Failed : Banned user login atttempt.",$uid,$user,1);
			$user_data["user_id"]=ANONYMOUS_USER;
			$user_data["user"]="Anonymous";
            $user_data["error_code"]="4";
			return $user_data;
		}

		$pwd=substr($pwd, 0, 4).$salt.substr($pwd, 4);
		$user_data["result"]=LOGGEDOUT;
		// Create a digest of the password collected from the challenge
		$password_digest = md5(trim($pwd));
		// Formulate the SQL to find the user
		$q = "  SELECT p_uuid  FROM users
                    WHERE user_name = '$user'
                    AND password = '$password_digest' and salt='{$salt}'";
		$res=$db->Execute($q);
		if(($res==null)||($res->EOF)){
			/* no result ,so return 1 ,which is  not a valid user_id , the calling
			 application can identify authentication was attempted,but failed
			 */
			//$error_code = 3;//echo "Login Failed : Invalid user name or password.";
			//shn_acl_log_msg("Login Failed : Invalid Password.",$uid,$user,1);
			$user_data["user_id"]=ANONYMOUS_USER;
			$user_data["user"]="Anonymous";
            $user_data["error_code"]="3";
			_shn_auth_lock_user($uid,$status);
			return $user_data;

		}else{
			if((($status=='locked')||($status=='try1')||($status=='try2'))){
				_shn_auth_activate_user($uid);
			}
			$user_data["user_id"]=$res->fields["p_uuid"];
			$user_data["user"]=$user;
			$user_data["result"]=LOGGEDIN;
			$global['welcome']=true;
			return $user_data;
		}
	}
}

