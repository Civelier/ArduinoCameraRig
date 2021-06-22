using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Reflection;
using System.Collections.ObjectModel;
using CameraRigController.FieldGrid.Editor.ViewModel;
using System.ComponentModel;
using System.Collections;
using System.Windows;

namespace CameraRigController.FieldGrid
{
    class FieldGridSupportedTypeInfo
    {
        public FieldGridSupportedTypeInfo(Type objectType, Type editorType, Type[] attributeTypes)
        {
            ObjectType = objectType;
            EditorType = editorType;
            AttributeTypes = attributeTypes;
        }

        public Type ObjectType { get; }
        public Type EditorType { get; }
        public Type[] AttributeTypes { get; }
    }
    public static class FieldGridUtillities
    {
        private static List<FieldGridSupportedTypeInfo> _supportedTypes { get; } = new List<FieldGridSupportedTypeInfo>();

        static FieldGridUtillities()
        {
            var types = Assembly.GetExecutingAssembly().GetTypes();
            foreach (var type in types)
            {
                var atts = type.GetCustomAttributes<FieldGridEditorAttribute>();
                var template = type.GetCustomAttribute<FieldgridTemplateAttribute>();
                if (template != null)
                {
                    Editor.FieldGridTemplateSelector.RegisterEditorResource(type, template.Provider);
                    foreach (var att in atts)
                    {
                        _supportedTypes.Add(new FieldGridSupportedTypeInfo(att.EditedObjectType, type, att.AttributeTypes));
                    }
                }
            }
        }

        public static ObservableCollection<EditorViewModelBase> ToVMCollection(object obj)
        {
            var collection = new ObservableCollection<EditorViewModelBase>();

            var properties = obj.GetType().GetProperties();
            var events = obj.GetType().GetEvents();

            MethodInfo objPropertyChanged;
            foreach (var e in events)
            {
                if (e.Name == "PropertyChanged")
                {
                    objPropertyChanged = e.GetAddMethod();
                    objPropertyChanged.Invoke(obj, new[] {(PropertyChangedEventHandler)((object sender, PropertyChangedEventArgs args) =>
                    {
                        var propertyEditor = collection.FirstOrDefault((p) => p.PropertyName == args.PropertyName);
                        if (propertyEditor == null) return;
                        var objProperty = properties.First((op) => op.Name == args.PropertyName);
                        var val = objProperty.GetValue(obj);
                        if (!propertyEditor.ObjectValue.Equals(val))
                        {
                            propertyEditor.ObjectValue = val;
                        }
                    }) });
                }
            }

            foreach (var p in properties)
            {
                var support = IsSupported(p.PropertyType);
                if (support != SupportInfo.NotSupported)
                {
                    var typeInfo = GetSupportedTypeInfo(p, support);
                    var editor = (EditorViewModelBase)Activator.CreateInstance(typeInfo.EditorType);
                    editor.PropertyName = p.Name;
                    editor.PropertyChanged += (sender, args) =>
                    {
                        if (args.PropertyName == "ObjectValue" && !(p.GetValue(obj)?.Equals(editor.ObjectValue) ?? false)) p.SetValue(obj, editor.ObjectValue);
                    };
                    editor.PropertyAttributes = p.GetCustomAttributes();
                    editor.ObjectValue = p.GetValue(obj);
                    editor.InjectedType = p.PropertyType;
                    editor.InitializeViewModel();
                    collection.Add(editor);
                }
            }



            return collection;
        }

        private static IEnumerable<FieldGridSupportedTypeInfo> GetAllSupportedTypeInfos(PropertyInfo memberInfo, SupportInfo support)
        {
            var result = new List<FieldGridSupportedTypeInfo>();
            if ((support & SupportInfo.ConcretelySupported) == SupportInfo.ConcretelySupported)
            {
                result.AddRange(_supportedTypes.FindAll((x) => x.ObjectType == memberInfo.PropertyType));
            }
            if ((support & SupportInfo.SubclassOfSupported) == SupportInfo.SubclassOfSupported)
            {
                result.AddRange(_supportedTypes.FindAll(x => x.ObjectType != typeof(object) &&
                    memberInfo.PropertyType.IsSubclassOf(x.ObjectType)));
            }
            if ((support & SupportInfo.ImplicitlySupported) == SupportInfo.ImplicitlySupported)
            {
                result.AddRange(_supportedTypes.FindAll(x => x.ObjectType == typeof(object)));
            }
            result.Sort((x, y) =>
            {
                var lengths = y.AttributeTypes.Length.CompareTo(x.AttributeTypes.Length);
                var xPriority = x.ObjectType == memberInfo.PropertyType ? 0 :
                    x.ObjectType != typeof(object) &&
                        memberInfo.PropertyType.IsSubclassOf(x.ObjectType) ? 1 :
                    x.ObjectType == typeof(object) ? 2 : 3;
                var yPriority = y.ObjectType == memberInfo.PropertyType ? 0 :
                    y.ObjectType != typeof(object) &&
                        memberInfo.PropertyType.IsSubclassOf(y.ObjectType) ? 1 :
                    y.ObjectType == typeof(object) ? 2 : 3;
                return lengths == 0 ? xPriority.CompareTo(yPriority) : lengths;
            });
            return result;
        }

        private static FieldGridSupportedTypeInfo GetSupportedTypeInfo(PropertyInfo memberInfo, SupportInfo support)
        {
            var infos = GetAllSupportedTypeInfos(memberInfo, support);
            
            return infos.FirstOrDefault((x) =>
            {
                var atts = memberInfo.GetCustomAttributes();
                foreach (var att in x.AttributeTypes)
                {
                    if (!atts.Any((a) => a.GetType() == att)) return false;
                }
                return true;
            });
        }

        public enum SupportInfo
        {
            /// <summary>
            /// Not supported
            /// </summary>
            NotSupported = 0b000,
            /// <summary>
            /// Supported from <see cref="object"/>
            /// </summary>
            ImplicitlySupported = 0b001,
            /// <summary>
            /// Supported by a concrete type
            /// </summary>
            ConcretelySupported = 0b010,
            /// <summary>
            /// Subclass a supported type (excluding <see cref="object"/>)
            /// </summary>
            SubclassOfSupported = 0b100,
        }

        public static SupportInfo IsSupported(Type type)
        {
            SupportInfo support = _supportedTypes.Exists(o => o.ObjectType == typeof(object))
                ? SupportInfo.ImplicitlySupported : SupportInfo.NotSupported;
            support |= _supportedTypes.Exists((o) => o.ObjectType == type) 
                ? SupportInfo.ConcretelySupported : SupportInfo.NotSupported;

            support |= _supportedTypes.Exists(o => type != typeof(object) && 
            type.IsSubclassOf(o.ObjectType))
                ? SupportInfo.SubclassOfSupported : SupportInfo.NotSupported;
            return support;
        }

        public static string NicifyName(string name)
        {
            var sb = new StringBuilder();
            bool numberSequence = false;
            bool capitalSequence = false;
            for (int i = 0; i < name.Length; i++)
            {
                if (char.IsLetter(name[i]))
                {
                    if (i == 0)
                    {
                        if (char.IsUpper(name[1])) capitalSequence = true;
                        else sb.Append(char.ToUpper(name[i]));
                    }
                    else
                    {
                        if (char.IsUpper(name[i]))
                        {
                            if (char.IsUpper(name[i-1]))
                            {
                                if (!capitalSequence)
                                {
                                    capitalSequence = true;
                                    
                                }
                                sb.Append(name[i - 1]);
                            }
                            else
                            {
                                if (numberSequence && i + 1 < name.Length && char.IsUpper(name[i + 1]))
                                {
                                    sb.Append(' ');
                                }
                                else
                                {
                                    sb.Append(' ');
                                    sb.Append(char.ToLower(name[i]));
                                }
                            }
                            if (i == name.Length - 1 && capitalSequence)
                            {
                                sb.Append(name[i]);
                            }
                        }
                        else
                        {
                            if (capitalSequence)
                            {
                                capitalSequence = false;
                                sb.Append(' ');
                                sb.Append(char.ToLower(name[i - 1]));
                            }
                            sb.Append(name[i]);
                        }
                    }
                    numberSequence = false;
                }
                else if (char.IsDigit(name[i]))
                {
                    if (capitalSequence) sb.Append(name[i - 1]);
                    if (!numberSequence)
                    {
                        numberSequence = true;
                        sb.Append(' ');
                    }
                    sb.Append(name[i]);
                    capitalSequence = false;
                }
            }
            return sb.ToString();
        }

        public static IEnumerable<LocalValueEntry> ToEnumerable(this LocalValueEnumerator enumerator)
        {
            while (enumerator.MoveNext()) yield return enumerator.Current;
        }
    }
}
